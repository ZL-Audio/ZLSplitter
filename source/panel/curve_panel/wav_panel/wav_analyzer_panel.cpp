// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "wav_analyzer_panel.hpp"

namespace zlpanel {
    WavAnalyzerPanel::WavAnalyzerPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)),
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
            g.setColour(base_.getTextColor());
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
        const auto bound = atomic_bound_.load();
        constexpr auto stereo_type = zldsp::analyzer::StereoType::kStereo;
        const auto time_length_idx = analyzer_time_length_ref_.load(std::memory_order::relaxed);

        {
            auto& sender{p_ref_.getController().getAnalyzerSender()};
            std::lock_guard lock{sender.getLock()};
            const auto sample_rate = sender.getSampleRate();
            const auto max_num_samples = sender.getMaxNumSamples();
            if (std::abs(sample_rate_ - sample_rate) > 0.1 ||
                max_num_samples_ != max_num_samples ||
                std::abs(time_length_idx_ - time_length_idx) > 0.1) {
                sample_rate_ = sample_rate;
                max_num_samples_ = max_num_samples;
                time_length_idx_ = time_length_idx;
                const auto time_idx = static_cast<size_t>(std::round(time_length_idx_));
                num_points_per_second_ = kNumPointsPerSecond[time_idx];
                num_samples_per_point_ = static_cast<int>(sample_rate_) / num_points_per_second_;
                time_length_ = zlstate::PMagTimeLength::kLength[time_idx];
                is_first_point_ = true;
                num_points_ = static_cast<size_t>(num_points_per_second_) * static_cast<size_t>(time_length_);
                second_per_point_ = static_cast<double>(time_length_) / static_cast<double>(num_points_);

                xs_.resize(num_points_ + 2);
                for (auto& y: {&min1s_, &max1s_, &min2s_, &max2s_}) {
                    y->resize(num_points_ + 2);
                }
            }

            auto& fifo{sender.getAbstractFIFO()};
            if (!is_first_point_) {
                // update ys
                while (next_time_stamp - start_time_ > second_per_point_) {
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
                            for (auto& y: {&min1s_, &max1s_, &min2s_, &max2s_}) {
                                std::ranges::fill(y->begin() + shift, y->end(), 0.f);
                            }
                        }
                    }
                    {
                        const auto too_many_missing = num_missing_points_ >= kPausedThreshold;
                        const auto scale = bound.getHeight() * 0.5f;
                        std::ranges::rotate(min1s_, min1s_.begin() + 1);
                        min1s_.back() = too_many_missing ? 0.f : minmax1_[0] * scale + scale;
                        std::ranges::rotate(max1s_, max1s_.begin() + 1);
                        max1s_.back() = too_many_missing ? 0.f : minmax1_[1] * scale + scale;
                        std::ranges::rotate(min2s_, min2s_.begin() + 1);
                        min2s_.back() = too_many_missing ? 0.f : minmax2_[0] * scale + scale;
                        std::ranges::rotate(max2s_, max2s_.begin() + 1);
                        max2s_.back() = too_many_missing ? 0.f : minmax2_[1] * scale + scale;
                    }
                    start_time_ += second_per_point_;
                }
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
                    for (auto& y: {&min1s_, &max1s_, &min2s_, &max2s_}) {
                        std::ranges::fill(y->begin(), y->end(), 0.f);
                    }
                }
            }
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
