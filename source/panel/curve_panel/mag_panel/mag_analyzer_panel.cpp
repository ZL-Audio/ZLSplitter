// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "mag_analyzer_panel.hpp"

#include <limits>
#include <mutex>

namespace zlpanel {
    MagAnalyzerPanel::MagAnalyzerPanel(PluginProcessor& p, zlgui::UIState& base) :
        p_ref_(p), base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)),
        analyzer_mag_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMagType::kID)),
        analyzer_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMagMinDB::kID)),
        analyzer_time_length_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMagTimeLength::kID)),
        analyzer_move_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMagMoveType::kID)) {
        constexpr auto preallocate_space = static_cast<int>(zlp::Controller<double>::kAnalyzerPointNum) * 3 + 1;
        for (auto* buffer : {&path1_, &path2_}) {
            for (auto& path : buffer->getBuffer()) {
                path.preallocateSpace(preallocate_space);
            }
        }
        consumer_id_ = transfer_buffer_.getMulticastFIFO().addConsumer();
        setInterceptsMouseClicks(false, false);
        lookAndFeelChanged();
    }

    MagAnalyzerPanel::~MagAnalyzerPanel() = default;

    void MagAnalyzerPanel::paint(juce::Graphics& g) {
        path1_.pull();
        path2_.pull();
        if (static_cast<zlp::PSplitType::SplitType>(
            std::round(split_type_ref_.load(std::memory_order::relaxed))) == zlp::PSplitType::kNone) {
            g.setColour(base_.gui.getTextColour());
            g.strokePath(path1_.getReader(),
                         juce::PathStrokeType(curve_thickness_,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        } else {
            const auto swap = swap_ref_.load(std::memory_order::relaxed) > .5f;
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput2Colour));
            g.strokePath(swap ? path1_.getReader() : path2_.getReader(),
                         juce::PathStrokeType(curve_thickness_,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput1Colour));
            g.strokePath(swap ? path2_.getReader() : path1_.getReader(),
                         juce::PathStrokeType(curve_thickness_,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
    }

    void MagAnalyzerPanel::resized() {
        atomic_bound_.store(getLocalBounds().toFloat());
        lookAndFeelChanged();
    }

    bool MagAnalyzerPanel::shouldResetHistory(const double next_time_stamp) const {
        const auto time_idx = static_cast<size_t>(std::round(
            analyzer_time_length_ref_.load(std::memory_order::relaxed)));
        const auto history_length = static_cast<double>(zlstate::PMagTimeLength::kLength[time_idx]);
        if (hasHistoryTimeDiscontinuity(next_time_stamp, previous_time_stamp_, history_length)) {
            return true;
        }
        return !is_first_point_ &&
            (next_time_stamp - start_time_ > history_length ||
             getHistoryAdvance(next_time_stamp, start_time_, second_per_point_, y1s_.size()).needs_reset);
    }

    void MagAnalyzerPanel::resetHistory(const double time_stamp) {
        is_first_point_ = true;
        start_time_ = time_stamp;
        motion_start_time_ = time_stamp;
        previous_time_stamp_ = time_stamp;
        roll_next_point_ = 0;
        num_missing_points_ = 0;
        too_much_samples_ = 0;
        db1_ = -240.f;
        db2_ = -240.f;
        const auto missing_y = std::fma(kMissingDB, db_to_y_scale_, db_to_y_bias_);
        std::ranges::fill(y1s_, missing_y);
        std::ranges::fill(y2s_, missing_y);
        for (auto* path : {&path1_, &path2_}) {
            path->getWriter().clear();
            path->publish();
        }
    }

    void MagAnalyzerPanel::run(const double next_time_stamp) {
        if (!isValidHistoryTimeStamp(next_time_stamp)) {
            return;
        }
        const auto bound = atomic_bound_.load();
        if (bound.isEmpty()) {
            return;
        }
        constexpr auto stereo_type = zldsp::analyzer::StereoType::kStereo;
        const auto mag_type = static_cast<zldsp::analyzer::MagType>(std::round(
            analyzer_mag_type_ref_.load(std::memory_order::relaxed)));
        const auto min_db = zlstate::PMagMinDB::kDBs[static_cast<size_t>(std::round(
            analyzer_min_db_ref_.load(std::memory_order::relaxed)))] / 6.f * 7.f;
        const auto time_length_idx = analyzer_time_length_ref_.load(std::memory_order::relaxed);
        const auto move_type = static_cast<MoveType>(std::round(
            analyzer_move_type_ref_.load(std::memory_order::relaxed)));
        const auto reset_history = reset_requested_ || shouldResetHistory(next_time_stamp);
        bool sender_changed{false};

        {
            auto& sender{p_ref_.getController().getAnalyzerSender()};
            const std::unique_lock lock{sender.getLock(), std::try_to_lock};
            if (!lock.owns_lock()) {
                return;
            }
            const auto sample_rate = sender.getSampleRate();
            const auto max_num_samples = sender.getMaxNumSamples();
            if (!std::isfinite(sample_rate) || sample_rate <= 0.0 || max_num_samples == 0) {
                return;
            }
            sender_changed = generation_ != sender.getGeneration() ||
                std::abs(sample_rate_ - sample_rate) > .1 || max_num_samples_ != max_num_samples;
            if (sender_changed) {
                generation_ = sender.getGeneration();
                sample_rate_ = sample_rate;
                max_num_samples_ = max_num_samples;
                transfer_buffer_.prepare(sample_rate_, max_num_samples_, {2, 2}, .5);
            }
            if (reset_history) {
                // discard stale samples before restarting history
                auto& sender_fifo{sender.getAbstractFIFO()};
                const auto num_ready = sender_fifo.getNumReady();
                (void)sender_fifo.prepareToRead(num_ready);
                sender_fifo.finishRead(num_ready);
                auto& fifo{transfer_buffer_.getMulticastFIFO()};
                const auto num_pending = fifo.getNumReady(consumer_id_);
                if (num_pending > 0) {
                    (void)fifo.prepareToRead(consumer_id_, num_pending);
                    fifo.finishRead(consumer_id_, num_pending);
                }
            } else {
                transfer_buffer_.processTransfer(sender.getAbstractFIFO(), sender.getSampleFIFOs());
            }
        }

        const auto settings_changed = sender_changed ||
            std::abs(time_length_idx_ - time_length_idx) > .1f || move_type_ != move_type || mag_type_ != mag_type;
        if (settings_changed) {
            time_length_idx_ = time_length_idx;
            move_type_ = move_type;
            mag_type_ = mag_type;
            const auto time_idx = static_cast<size_t>(std::round(time_length_idx_));
            time_length_ = zlstate::PMagTimeLength::kLength[time_idx];
            const auto rounded_sample_rate = static_cast<int>(std::round(sample_rate_));
            num_points_per_second_ = chooseHistoryPointsPerSecond(
                rounded_sample_rate, kNumPointsPerSecond[time_idx], static_cast<int>(std::round(time_length_)));
            num_samples_per_point_ = std::max(1, rounded_sample_rate / num_points_per_second_);
            is_first_point_ = true;
            num_points_ = static_cast<size_t>(num_points_per_second_) * static_cast<size_t>(time_length_);
            second_per_point_ = static_cast<double>(num_samples_per_point_) / sample_rate_;
            const auto history_size = move_type_ == MoveType::kRoll ? num_points_ : num_points_ + 2;
            xs_.resize(history_size);
            y1s_.resize(history_size);
            y2s_.resize(history_size);
        }
        updateYMapping(bound, min_db);
        if (reset_history || settings_changed) {
            resetHistory(next_time_stamp);
        }
        reset_requested_ = false;
        previous_time_stamp_ = next_time_stamp;
        const auto missing_y = std::fma(kMissingDB, db_to_y_scale_, db_to_y_bias_);

        auto& fifo{transfer_buffer_.getMulticastFIFO()};
        if (!is_first_point_) {
            const auto advance = getHistoryAdvance(next_time_stamp, start_time_, second_per_point_, y1s_.size());
            if (advance.needs_reset) {
                resetHistory(next_time_stamp);
                const auto num_ready = fifo.getNumReady(consumer_id_);
                if (num_ready > 0) {
                    (void)fifo.prepareToRead(consumer_id_, num_ready);
                    fifo.finishRead(consumer_id_, num_ready);
                }
                return;
            }
            // update ys
            for (size_t point = 0; point < advance.num_points; ++point) {
                if (fifo.getNumReady(consumer_id_) >= num_samples_per_point_) {
                    const auto range = fifo.prepareToRead(consumer_id_, num_samples_per_point_);
                    db1_ = zldsp::analyzer::MagReceiver::calculate(
                        range, transfer_buffer_.getSampleFIFOs()[0], mag_type, stereo_type);
                    db2_ = zldsp::analyzer::MagReceiver::calculate(
                        range, transfer_buffer_.getSampleFIFOs()[1], mag_type, stereo_type);
                    fifo.finishRead(consumer_id_, num_samples_per_point_);
                    num_missing_points_ = 0;
                } else {
                    if (num_missing_points_ < kPausedThreshold) {
                        num_missing_points_ += 1;
                    } else if (num_missing_points_ == kPausedThreshold) {
                        const auto shift = static_cast<ptrdiff_t>(y1s_.size() - static_cast<size_t>(kPausedThreshold));
                        std::ranges::fill(y1s_.begin() + shift, y1s_.end(), missing_y);
                        std::ranges::fill(y2s_.begin() + shift, y2s_.end(), missing_y);
                    }
                }
                const auto too_many_missing = num_missing_points_ >= kPausedThreshold;
                std::ranges::rotate(y1s_, y1s_.begin() + 1);
                y1s_.back() = too_many_missing ? missing_y : std::fma(db1_, db_to_y_scale_, db_to_y_bias_);
                std::ranges::rotate(y2s_, y2s_.begin() + 1);
                y2s_.back() = too_many_missing ? missing_y : std::fma(db2_, db_to_y_scale_, db_to_y_bias_);
                if (move_type_ == MoveType::kRoll) {
                    roll_next_point_ = (roll_next_point_ + 1) % xs_.size();
                }
            }
            start_time_ = advance.time_stamp;
            // if too much samples
            const auto num_ready = fifo.getNumReady(consumer_id_);
            const auto threshold = 2 * std::max(static_cast<int>(max_num_samples_), num_samples_per_point_);
            if (num_ready > threshold) {
                too_much_samples_ += (num_ready - threshold) / num_samples_per_point_;
                if (too_much_samples_ > kTooMuchResetThreshold) {
                    (void)fifo.prepareToRead(consumer_id_, num_ready - threshold);
                    fifo.finishRead(consumer_id_, num_ready - threshold);
                    too_much_samples_ = 0;
                }
            } else {
                too_much_samples_ = 0;
            }
        } else if (fifo.getNumReady(consumer_id_) >= num_samples_per_point_) {
            is_first_point_ = false;
            start_time_ = next_time_stamp;
            motion_start_time_ = next_time_stamp;
            std::ranges::fill(y1s_, missing_y);
            std::ranges::fill(y2s_, missing_y);
        }

        if (!is_first_point_) {
            updateXs(bound, next_time_stamp);
            updatePaths();
        } else {
            path1_.getWriter().clear();
            path2_.getWriter().clear();
        }
        path1_.publish();
        path2_.publish();
    }

    void MagAnalyzerPanel::updateXs(const juce::Rectangle<float> bound, const double next_time_stamp) {
        if (move_type_ == MoveType::kRoll) {
            const auto delta_x = static_cast<double>(bound.getWidth()) / static_cast<double>(xs_.size() - 1);
            for (size_t i = 0; i < xs_.size(); ++i) {
                xs_[i] = bound.getX() + static_cast<float>(
                    static_cast<double>((roll_next_point_ + i) % xs_.size()) * delta_x);
            }
            return;
        }

        const auto length = static_cast<double>(time_length_);
        const auto x_scale = static_cast<double>(bound.getWidth()) / length;
        const auto delta_x = second_per_point_ * x_scale;
        auto x0 = static_cast<double>(bound.getX()) - (next_time_stamp - start_time_) * x_scale;
        if (move_type_ == MoveType::kSlow) {
            constexpr auto fast_length = .5;
            constexpr auto restart_position = .25;
            const auto slow_length = length - fast_length;
            const auto phase = std::fmod(std::max(0.0, next_time_stamp - motion_start_time_), length);
            constexpr auto sweep = 1.0 - restart_position;
            const auto head = phase < slow_length
                ? restart_position + sweep * phase / slow_length
                : 1.0 - sweep * (phase - slow_length) / fast_length;
            x0 += head * static_cast<double>(bound.getWidth()) -
                static_cast<double>(xs_.size() - 1) * delta_x;
        }
        for (size_t i = 0; i < xs_.size(); ++i) {
            xs_[i] = static_cast<float>(x0);
            x0 += delta_x;
        }
    }

    void MagAnalyzerPanel::updateYMapping(const juce::Rectangle<float> bound, const float min_db) {
        const auto next_scale = std::max(bound.getHeight(), 1.f) / min_db;
        const auto next_bias = bound.getY();
        if (is_y_mapping_initialized_ && !is_first_point_ &&
            std::abs(db_to_y_scale_) > std::numeric_limits<float>::epsilon()) {
            const auto scale_changed = std::abs(next_scale - db_to_y_scale_) > 1e-6f;
            const auto bias_changed = std::abs(next_bias - db_to_y_bias_) > 1e-6f;
            if (scale_changed || bias_changed) {
                const auto scale = next_scale / db_to_y_scale_;
                const auto bias = std::fma(-db_to_y_bias_, scale, next_bias);
                zldsp::vector::fma(y1s_.data(), scale, bias, y1s_.size());
                zldsp::vector::fma(y2s_.data(), scale, bias, y2s_.size());
            }
        }
        db_to_y_scale_ = next_scale;
        db_to_y_bias_ = next_bias;
        is_y_mapping_initialized_ = true;
    }

    void MagAnalyzerPanel::updatePaths() {
        auto& next_path1{path1_.getWriter()};
        auto& next_path2{path2_.getWriter()};
        next_path1.clear();
        next_path2.clear();
        next_path1.startNewSubPath(xs_[0], y1s_[0]);
        next_path2.startNewSubPath(xs_[0], y2s_[0]);
        for (size_t i = 1; i < xs_.size(); ++i) {
            if (xs_[i] < xs_[i - 1]) {
                next_path1.startNewSubPath(xs_[i], y1s_[i]);
                next_path2.startNewSubPath(xs_[i], y2s_[i]);
                continue;
            }
            next_path1.lineTo(xs_[i], y1s_[i]);
            next_path2.lineTo(xs_[i], y2s_[i]);
        }
    }

    void MagAnalyzerPanel::lookAndFeelChanged() {
        curve_thickness_ = base_.gui.getFontSize() * .2f * base_.getMagCurveThickness();
    }
}
