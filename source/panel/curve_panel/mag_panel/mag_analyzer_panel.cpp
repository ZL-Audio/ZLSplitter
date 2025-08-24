// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "mag_analyzer_panel.hpp"

namespace zlpanel {
    MagAnalyzerPanel::MagAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
          swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)),
          fft_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTMinDB::kID)),
          mag_analyzer_ref_(p.getController().getMagAnalyzer()) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::Controller<double>::kAnalyzerPointNum) * 3 + 1;
        for (auto &path: {&out_path1_, &out_path2_}) {
            path->preallocateSpace(preallocateSpace);
        }

        for (auto &ID: kNAIDs) {
            p_ref_.na_parameters_.addParameterListener(ID, this);
            parameterChanged(ID, p_ref_.na_parameters_.getRawParameterValue(ID)->load(std::memory_order::relaxed));
        }

        mag_analyzer_ref_.setToReset();

        setInterceptsMouseClicks(false, false);
    }

    MagAnalyzerPanel::~MagAnalyzerPanel() {
        for (auto &ID: kNAIDs) {
            p_ref_.na_parameters_.removeParameterListener(ID, this);
        }
    }

    void MagAnalyzerPanel::paint(juce::Graphics &g) {
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto thickness = base_.getFontSize() * .2f * base_.getFFTCurveThickness();
        if (static_cast<zlp::PSplitType::SplitType>(
                std::round(split_type_ref_.load(std::memory_order_relaxed))) == zlp::PSplitType::kNone) {
            g.setColour(base_.getTextColor());
            g.strokePath(out_path1_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        } else {
            const auto swap = swap_ref_.load(std::memory_order::relaxed) > .5f;
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput2Colour));
            g.strokePath(swap ? out_path1_ : out_path2_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput1Colour));
            g.strokePath(swap ? out_path2_ : out_path1_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
    }

    void MagAnalyzerPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        constexpr auto pad_p = 1.f / static_cast<float>(zlp::Controller<double>::kAnalyzerPointNum - 1);
        const auto pad = std::max(bound.getWidth() * pad_p, 1.f);
        bound = bound.withWidth(bound.getWidth() + pad);
        atomic_bound_.store(bound);
        lookAndFeelChanged();
    }

    void MagAnalyzerPanel::run(const double next_time_stamp) {
        const auto current_bound = atomic_bound_.load();
        if (to_reset_path_.exchange(false, std::memory_order::acquire)
            || next_time_stamp - current_time > 0.5) {
            is_first_point_ = true;
        }
        current_time = next_time_stamp;
        if (is_first_point_) {
            auto [actual_delta, to_reset_shift] = mag_analyzer_ref_.run(1, 0);
            if (actual_delta > 0) {
                is_first_point_ = false;
                current_count_ = 0.;
                start_time_ = next_time_stamp;
                mag_analyzer_ref_.createPath(xs_, {out1_ys_, out2_ys_},
                                             current_bound.getWidth(), current_bound.getHeight(), 0.f,
                                             analyzer_min_db_.load(std::memory_order::relaxed), 0.f);
                updatePaths(current_bound);
            }
        } else {
            const auto c_num_per_second = num_per_second_.load(std::memory_order::relaxed);
            const auto target_count = (next_time_stamp - start_time_) * c_num_per_second;
            const auto tolerance = std::max(target_count * 1.5, 1.0);
            const auto target_delta = target_count - current_count_;
            auto [actual_delta, to_reset_shift] = mag_analyzer_ref_.run(
                static_cast<int>(std::floor(target_delta)),
                static_cast<int>(std::round(tolerance)));
            current_count_ += static_cast<double>(actual_delta);

            if (to_reset_shift) {
                start_time_ = next_time_stamp;
                current_count_ = 0.;
            }

            const auto shift = to_reset_shift ? 0.0 : target_count - current_count_;
            mag_analyzer_ref_.createPath(xs_, {out1_ys_, out2_ys_},
                                         current_bound.getWidth(), current_bound.getHeight(),
                                         static_cast<float>(shift),
                                         analyzer_min_db_.load(std::memory_order::relaxed), 0.f);
            updatePaths(current_bound);
        } {
            std::lock_guard<std::mutex> lock{mutex_};
            out_path1_.swapWithPath(next_out_path1_);
            out_path2_.swapWithPath(next_out_path2_);
        }
    }

    void MagAnalyzerPanel::updatePaths(const juce::Rectangle<float>) {
        next_out_path1_.clear();
        next_out_path2_.clear();

        next_out_path1_.startNewSubPath(xs_[0], out1_ys_[0]);
        next_out_path2_.startNewSubPath(xs_[0], out2_ys_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            next_out_path1_.lineTo(xs_[i], out1_ys_[i]);
            next_out_path2_.lineTo(xs_[i], out2_ys_[i]);
        }
    }

    void MagAnalyzerPanel::parameterChanged(const juce::String &parameter_id, const float new_value) {
        if (parameter_id == zlstate::PMagType::kID) {
            mag_analyzer_ref_.setMagType(static_cast<zldsp::analyzer::MagType>(std::round(new_value)));
        } else if (parameter_id == zlstate::PMagMinDB::kID) {
            analyzer_min_db_.store(zlstate::PMagMinDB::getMinDBFromIndex(new_value), std::memory_order::relaxed);
        } else if (parameter_id == zlstate::PMagTimeLength::kID) {
            setTimeLength(zlstate::PMagTimeLength::getTimeLengthFromIndex(new_value));
        }
    }
}
