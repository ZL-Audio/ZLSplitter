// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "fft_analyzer_panel.hpp"

namespace zlpanel {
    FFTAnalyzerPanel::FFTAnalyzerPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : p_ref_(processor),
          base_(base),
          split_type_ref_(*p_ref_.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
          swap_ref_(*p_ref_.parameters_.getRawParameterValue(zlp::PSwap::kID)) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::Controller<double>::kAnalyzerPointNum) * 3 + 1;
        for (auto &path: {&out_path1_, &out_path2_, &next_out_path1_, &next_out_path2_}) {
            path->preallocateSpace(preallocateSpace);
        }

        setInterceptsMouseClicks(false, false);
    }

    FFTAnalyzerPanel::~FFTAnalyzerPanel() = default;

    void FFTAnalyzerPanel::paint(juce::Graphics &g) {
        if (skip_next_repaint_) {
            skip_next_repaint_ = false;
            return;
        }
        const std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        if (swap_ref_.load(std::memory_order::relaxed) < .5f) {
            g.setColour(base_.getColorMap1(1));
            g.strokePath(out_path2_, juce::PathStrokeType(base_.getFontSize() * .15f,
                                                          juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
            g.setColour(base_.getColorMap1(0));
            g.strokePath(out_path1_, juce::PathStrokeType(base_.getFontSize() * .15f,
                                                          juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
        } else {
            g.setColour(base_.getColorMap1(1));
            g.strokePath(out_path1_, juce::PathStrokeType(base_.getFontSize() * .15f,
                                                          juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
            g.setColour(base_.getColorMap1(0));
            g.strokePath(out_path2_, juce::PathStrokeType(base_.getFontSize() * .15f,
                                                          juce::PathStrokeType::curved,
                                                          juce::PathStrokeType::rounded));
        }
    }

    void FFTAnalyzerPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound);
        skip_next_repaint_ = true;
    }

    void FFTAnalyzerPanel::run() {
        auto &analyzer{p_ref_.getController().getFFTAnalyzer()};
        if (!analyzer.getLock().try_lock()) {
            return;
        }
        const auto akima_reset_flag = analyzer.run();
        const size_t n = analyzer.getInterplotSize();
        if (akima_reset_flag || n != xs_.size()) {
            xs_.resize(n);
            y1_.resize(n);
            y2_.resize(n);
            width_ = -1.f;
        }

        const auto bound = atomic_bound_.load();
        // re-calculate xs if width changes
        if (std::abs(bound.getWidth() - width_) > 1e-3f) {
            width_ = bound.getWidth();
            analyzer.createPathXs(xs_, width_);
        }
        analyzer.createPathYs({std::span{y1_}, std::span{y2_}}, bound.getHeight());
        analyzer.getLock().unlock();

        next_out_path1_.clear();
        next_out_path1_.startNewSubPath(bound.getBottomLeft());
        next_out_path2_.clear();
        next_out_path2_.startNewSubPath(bound.getBottomLeft());
        for (size_t i = 0; i < xs_.size(); ++i) {
            if (std::isfinite(y1_[i])) {
                next_out_path1_.lineTo(xs_[i], y1_[i]);
            }
            if (std::isfinite(y2_[i])) {
                next_out_path2_.lineTo(xs_[i], y2_[i]);
            }
        }

        std::lock_guard<std::mutex> lock{mutex_};
        out_path1_.swapWithPath(next_out_path1_);
        out_path2_.swapWithPath(next_out_path2_);
    }
}
