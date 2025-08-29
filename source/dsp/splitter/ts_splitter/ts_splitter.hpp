// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <numbers>

#include "median_filter.hpp"
#include "../../filter/fir_filter/fir_base.hpp"
#include "../../delay/integer_delay.hpp"

namespace zldsp::splitter {
    template<typename FloatType>
    class TSSplitter final : public zldsp::filter::FIRBase<FloatType, 10> {
    public:
        TSSplitter() = default;

        void prepare(const double sample_rate,
                     [[maybe_unused]] const size_t num_channels,
                     const size_t max_num_samples) {
            zldsp::filter::FIRBase<FloatType, 10>::prepare(sample_rate, 1);
            const auto delay_num = static_cast<int>(this->fft_size_ + this->hop_size_ * kTimeHalfMedianWindowsSize);
            delay_.prepare(sample_rate, max_num_samples, 1,
                           static_cast<FloatType>(delay_num + 2) / static_cast<FloatType>(sample_rate));
            delay_.setDelayInSamples(delay_num);
        }

        void process(FloatType * in_buffer,
                     FloatType * transient_buffer,
                     FloatType * steady_buffer,
                     const size_t num_samples) {
            // copy in buffer to steady buffer and delay it
            zldsp::vector::copy(steady_buffer, in_buffer, num_samples);
            delay_span_[0] = steady_buffer;
            delay_.process(delay_span_, num_samples);
            // transient split
            zldsp::vector::copy(transient_buffer, in_buffer, num_samples);
            fft_span_[0] = transient_buffer;
            zldsp::filter::FIRBase<FloatType, 10>::process(fft_span_, num_samples);
            // subtract transient buffer from steady buffer
            auto transient_v = kfr::make_univector(transient_buffer, num_samples);
            auto steady_v = kfr::make_univector(steady_buffer, num_samples);
            steady_v = steady_v - transient_v;
        }

        int getTSLatency() const { return delay_.getDelayInSamples(); }

        void setBalance(const float x) {
            balance_.store(std::pow(16.f, x - 0.75f), std::memory_order::relaxed);
        }

        void setSmooth(const float x) {
            smooth_.store(x, std::memory_order::relaxed);
        }

        void setHold(const float x) {
            hold_.store((32.f - std::pow(32.f, 1.f - x)) / 31.f * 0.75f + 0.24f, std::memory_order::relaxed);
        }

        void setSeparation(const float x) {
            separation_.store(std::exp(x * 4.f) - 1.f, std::memory_order::relaxed);
        }

    private:
        static constexpr size_t kFreqMedianWindowsSize = 5;
        static constexpr size_t kFreqHalfMedianWindowsSize = kFreqMedianWindowsSize / 2;
        static constexpr size_t kTimeMedianWindowsSize = 5;
        static constexpr size_t kTimeHalfMedianWindowsSize = kTimeMedianWindowsSize / 2;

        std::array<FloatType *, 1> delay_span_;
        zldsp::delay::IntegerDelay<FloatType> delay_;
        // extra fft working space
        std::array<FloatType *, 1> fft_span_;
        size_t fft_line_pos_ = 0;
        std::array<std::vector<float>, kTimeHalfMedianWindowsSize + 1> fft_lines_;
        kfr::univector<float> magnitude_;
        // median calculators
        std::vector<HeapFilter<float, kTimeMedianWindowsSize> > time_median_{};
        HeapFilter<float, kFreqMedianWindowsSize> freq_median_{};
        // portion holders
        kfr::univector<float> mask_;
        // separation factor
        std::atomic<float> balance_{5.f}, separation_{1.f}, hold_{0.9f}, smooth_{.5f};
        float c_balance_{0.f}, c_separation_{0.f}, c_hold_{0.f}, c_smooth_{0.f};

        void setOrder(const size_t, const size_t order) override {
            zldsp::filter::FIRBase<FloatType, 10>::setFFTOrder(1, order);
            // set fft data lines
            fft_line_pos_ = 0;
            for (auto &line : fft_lines_) {
                line.resize(this->fft_data_.size());
                std::fill(line.begin(), line.end(), 0.f);
            }
            magnitude_.resize(this->num_bins_);
            time_median_.resize(this->num_bins_);
            mask_.resize(this->num_bins_);
        }

        void processSpectrum() override {
            // calculate magnitude
            for (size_t i = 0; i < this->num_bins_; ++i) {
                const auto re = this->fft_data_[2 * i];
                const auto im = this->fft_data_[2 * i + 1];
                magnitude_[i] = std::sqrt(re * re + im * im);
            }
            // calculate median and masks
            freq_median_.clear();
            for (size_t i = 0; i < kFreqHalfMedianWindowsSize; ++i) {
                freq_median_.insert(magnitude_[0]);
            }
            for (size_t i = 0; i < kFreqHalfMedianWindowsSize; ++i) {
                freq_median_.insert(magnitude_[i]);
            }
            // load parameters
            c_balance_ = balance_.load(std::memory_order::relaxed);
            c_separation_ = separation_.load(std::memory_order::relaxed);
            c_hold_ = hold_.load(std::memory_order::relaxed);
            c_smooth_ = smooth_.load(std::memory_order::relaxed);
            // calculate mask
            for (size_t i = 0; i < this->num_bins_ - kFreqHalfMedianWindowsSize; ++i) {
                freq_median_.insert(magnitude_[i + kFreqHalfMedianWindowsSize]);
                time_median_[i].insert(magnitude_[i]);
                const auto c_mask = calculatePortion(freq_median_.getMedian(), time_median_[i].getMedian());
                mask_[i] = std::max(mask_[i] * c_hold_, c_mask);
            }
            // retrieve fft data
            zldsp::vector::copy(fft_lines_[fft_line_pos_].data(), this->fft_data_.data(), this->fft_data_.size());
            fft_line_pos_ = (fft_line_pos_ + 1) % (kTimeHalfMedianWindowsSize + 1);
            zldsp::vector::copy(this->fft_data_.data(), fft_lines_[fft_line_pos_].data(), this->fft_data_.size());
            // apply mask
            auto mask_mean = kfr::mean(mask_);
            mask_mean = std::clamp((mask_mean - 0.5f) * std::sqrt(c_separation_), -.5f, .5f) + .5f;
            for (size_t i = 0; i < this->num_bins_; ++i) {
                const auto i1 = i * 2;
                const auto i2 = i1 + 1;
                const auto bin_mask = (mask_mean - mask_[i]) * c_smooth_ + mask_[i];
                this->fft_data_[i1] *= bin_mask;
                this->fft_data_[i2] *= bin_mask;
            }
        }

        float calculatePortion(const float transient_weight, const float steady_weight) const {
            const auto t = transient_weight * c_balance_;
            const auto s = steady_weight;
            const auto tt = t * t;
            const auto ss = s * s;
            const auto p = tt / std::max(tt + ss, 0.00000001f);
            return std::clamp((p - 0.5f) * c_separation_, -5.f, .5f) + .5f;
        }
    };
}
