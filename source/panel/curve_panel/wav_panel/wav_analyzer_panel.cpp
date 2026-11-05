// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "wav_analyzer_panel.hpp"
#include "../../helper/history_timing.hpp"

namespace zlpanel {
    WavAnalyzerPanel::WavAnalyzerPanel(PluginProcessor& p, zlgui::UIState& base) :
        p_ref_(p), base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)),
        analyzer_max_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PWavMaxDB::kID)),
        analyzer_time_length_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMagTimeLength::kID)) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::Controller<double>::kAnalyzerPointNum) * 3 + 1;
        for (auto& path : {&path1_, &path2_}) {
            path->preallocateSpace(preallocateSpace);
        }

        setInterceptsMouseClicks(false, false);
    }

    WavAnalyzerPanel::~WavAnalyzerPanel() = default;

    void WavAnalyzerPanel::paint(juce::Graphics& g) {
        const std::unique_lock lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        if (static_cast<zlp::PSplitType::SplitType>(
            std::round(split_type_ref_.load(std::memory_order_relaxed))) == zlp::PSplitType::kNone) {
            g.setColour(base_.gui.getTextColour());
            g.fillPath(path1_);
        } else {
            const auto swap = swap_ref_.load(std::memory_order::relaxed) > .5f;
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput2Colour).withAlpha(.25f));
            g.fillPath(swap ? path1_ : path2_);
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput1Colour).withAlpha(.75f));
            g.fillPath(swap ? path2_ : path1_);
        }
    }

    void WavAnalyzerPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        constexpr auto pad_p = 1.f / static_cast<float>(zlp::Controller<double>::kAnalyzerPointNum - 1);
        const auto pad = std::max(bound.getWidth() * pad_p, 1.f);
        bound = bound.withWidth(bound.getWidth() + pad);
        atomic_bound_.store(bound);
        lookAndFeelChanged();
    }

    void WavAnalyzerPanel::run(const double next_time_stamp) {
        if (!isValidHistoryTimeStamp(next_time_stamp)) { return; }
        const auto bound = atomic_bound_.load();
        if (bound.isEmpty()) { return; }
        constexpr auto stereo_type = zldsp::analyzer::StereoType::kStereo;
        const auto time_length_idx = analyzer_time_length_ref_.load(std::memory_order::relaxed);

        {
            auto& sender{p_ref_.getController().getAnalyzerSender()};
            const std::unique_lock lock{sender.getLock(), std::try_to_lock};
            if (!lock.owns_lock()) { return; }
            const auto sample_rate = sender.getSampleRate();
            const auto max_num_samples = sender.getMaxNumSamples();
            if (!std::isfinite(sample_rate) || sample_rate <= 0.0) { return; }
            if (generation_ != sender.getGeneration() || std::abs(sample_rate_ - sample_rate) > 0.1 ||
                max_num_samples_ != max_num_samples ||
                std::abs(time_length_idx_ - time_length_idx) > 0.1) {
                generation_ = sender.getGeneration();
                sample_rate_ = sample_rate;
                max_num_samples_ = max_num_samples;
                time_length_idx_ = time_length_idx;
                const auto time_idx = static_cast<size_t>(std::round(time_length_idx_));
                time_length_ = zlstate::PMagTimeLength::kLength[time_idx];
                const auto rounded_sample_rate = static_cast<int>(std::round(sample_rate_));
                num_points_per_second_ = chooseHistoryPointsPerSecond(
                    rounded_sample_rate, kNumPointsPerSecond[time_idx], static_cast<int>(time_length_));
                num_samples_per_point_ = std::max(1, rounded_sample_rate / num_points_per_second_);
                is_first_point_ = true;
                num_missing_points_ = 0;
                too_much_samples_ = 0;
                num_points_ = static_cast<size_t>(num_points_per_second_) * static_cast<size_t>(time_length_);
                second_per_point_ = static_cast<double>(num_samples_per_point_) / sample_rate_;

                xs_.resize(num_points_ + 2);
                for (auto& y : {&min1s_, &max1s_, &min2s_, &max2s_}) {
                    y->resize(num_points_ + 2);
                }
            }

            const auto max_db = zlstate::PWavMaxDB::getMaxDBFromIndex(
                analyzer_max_db_ref_.load(std::memory_order::relaxed));
            const auto next_scale = bound.getHeight() * .5f / zldsp::chore::decibelsToGain(max_db);
            const auto next_bias = bound.getHeight() * .5f;
            if (!is_first_point_ && y_scale_ != 0.f &&
                (std::abs(next_scale - y_scale_) > 1e-6f || std::abs(next_bias - y_bias_) > 1e-6f)) {
                const auto ratio = next_scale / y_scale_;
                const auto bias = std::fma(-y_bias_, ratio, next_bias);
                for (auto* values : {&min1s_, &max1s_, &min2s_, &max2s_}) {
                    zldsp::vector::fma(values->data(), ratio, bias, values->size());
                }
            }
            y_scale_ = next_scale;
            y_bias_ = next_bias;
            auto& fifo{sender.getAbstractFIFO()};
            const auto advance = getHistoryAdvance(next_time_stamp, start_time_, second_per_point_, num_points_);
            if (!is_first_point_ && advance.needs_reset) {
                is_first_point_ = true;
                num_missing_points_ = 0;
                too_much_samples_ = 0;
                const auto pending = fifo.getNumReady();
                (void)fifo.prepareToRead(pending);
                fifo.finishRead(pending);
            }
            if (!is_first_point_) {
                // update ys
                for (size_t point = 0; point < advance.num_points; ++point) {
                    // if not enough samples
                    if (fifo.getNumReady() >= num_samples_per_point_) {
                        const auto range = fifo.prepareToRead(num_samples_per_point_);
                        minmax1_ = zldsp::analyzer::WaveReceiver::calculate(
                            range, sender.getSampleFIFOs()[0], stereo_type);
                        minmax2_ = zldsp::analyzer::WaveReceiver::calculate(
                            range, sender.getSampleFIFOs()[1], stereo_type);
                        fifo.finishRead(num_samples_per_point_);
                        num_missing_points_ = 0;
                    } else {
                        if (num_missing_points_ < kPausedThreshold) {
                            num_missing_points_ += 1;
                        } else if (num_missing_points_ == kPausedThreshold) {
                            const auto shift = static_cast<ptrdiff_t>(
                                xs_.size() - static_cast<size_t>(kPausedThreshold));
                            for (auto& y : {&min1s_, &max1s_, &min2s_, &max2s_}) {
                                std::ranges::fill(y->begin() + shift, y->end(), bound.getHeight() * .5f);
                            }
                        }
                    }
                    {
                        const auto too_many_missing = num_missing_points_ >= kPausedThreshold;
                        const auto scale = y_scale_;
                        const auto bias = y_bias_;
                        std::ranges::rotate(min1s_, min1s_.begin() + 1);
                        min1s_.back() = too_many_missing ? bias : minmax1_[0] * scale + bias;
                        std::ranges::rotate(max1s_, max1s_.begin() + 1);
                        max1s_.back() = too_many_missing ? bias : minmax1_[1] * scale + bias;
                        std::ranges::rotate(min2s_, min2s_.begin() + 1);
                        min2s_.back() = too_many_missing ? bias : minmax2_[0] * scale + bias;
                        std::ranges::rotate(max2s_, max2s_.begin() + 1);
                        max2s_.back() = too_many_missing ? bias : minmax2_[1] * scale + bias;
                    }
                }
                start_time_ = advance.time_stamp;
                // if too much samples
                const auto num_ready = fifo.getNumReady();
                const auto threshold = 2 * std::max(static_cast<int>(max_num_samples_), num_samples_per_point_);
                if (num_ready > threshold) {
                    too_much_samples_ += (num_ready - threshold) / num_samples_per_point_;
                    if (too_much_samples_ > kTooMuchResetThreshold) {
                        (void)fifo.prepareToRead(num_ready - threshold);
                        fifo.finishRead(num_ready - threshold);
                        too_much_samples_ = 0;
                    }
                } else {
                    too_much_samples_ = 0;
                }
            } else {
                if (fifo.getNumReady() >= num_samples_per_point_) {
                    is_first_point_ = false;
                    start_time_ = next_time_stamp;
                    for (auto& y : {&min1s_, &max1s_, &min2s_, &max2s_}) {
                        std::ranges::fill(y->begin(), y->end(), bound.getHeight() * .5f);
                    }
                }
            }
        }
        if (is_first_point_) {
            std::lock_guard lock{mutex_};
            path1_.clear();
            path2_.clear();
            return;
        }
        // update xs
        if (!is_first_point_) {
            const auto x_scale = static_cast<double>(bound.getWidth()) / static_cast<double>(time_length_);
            auto x0 = -(next_time_stamp - start_time_) * x_scale;
            const auto delta_x = second_per_point_ * x_scale;
            for (size_t i = 0; i < xs_.size(); ++i) {
                xs_[i] = static_cast<float>(x0);
                x0 += delta_x;
            }
        }
        if (!is_first_point_) {
            updatePaths(bound);
            std::lock_guard lock{mutex_};
            path1_.swapWithPath(next_path1_);
            path2_.swapWithPath(next_path2_);
        }
    }

    void WavAnalyzerPanel::updatePaths(const juce::Rectangle<float>) {
        next_path1_.clear();
        next_path2_.clear();

        next_path1_.startNewSubPath(xs_[0], min1s_[0]);
        next_path2_.startNewSubPath(xs_[0], min2s_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            next_path1_.lineTo(xs_[i], min1s_[i]);
            next_path2_.lineTo(xs_[i], min2s_[i]);
        }
        for (size_t i = xs_.size(); i > 0; --i) {
            next_path1_.lineTo(xs_[i - 1], max1s_[i - 1]);
            next_path2_.lineTo(xs_[i - 1], max2s_[i - 1]);
        }

        next_path1_.closeSubPath();
        next_path2_.closeSubPath();
    }
}
