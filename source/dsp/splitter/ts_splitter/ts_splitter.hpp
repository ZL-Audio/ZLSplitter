// Copyright (C) 2026 - zsliu98
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

        void process(FloatType *in_buffer,
                     FloatType *transient_buffer,
                     FloatType *steady_buffer,
                     const size_t num_samples) {
            const std::array<FloatType*, 1> input{in_buffer};
            delay_span_[0] = steady_buffer;
            delay_.process(input, delay_span_, num_samples);
            fft_span_[0] = transient_buffer;
            zldsp::filter::FIRBase<FloatType, 10>::process(input, fft_span_, num_samples);
            // subtract transient buffer from steady buffer
            zldsp::vector::sub(steady_buffer, transient_buffer, num_samples);
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
        std::array<typename zldsp::filter::FIRBase<FloatType, 10>::Spectrum, kTimeHalfMedianWindowsSize> fft_lines_;
        zldsp::vector::aligned_vector<float> magnitude_;
        // median calculators
        std::vector<HeapFilter<float, kTimeMedianWindowsSize>> time_median_{};
        HeapFilter<float, kFreqMedianWindowsSize> freq_median_{};
        // portion holders
        zldsp::vector::aligned_vector<float> mask_;
        // separation factor
        std::atomic<float> balance_{5.f}, separation_{1.f}, hold_{0.9f}, smooth_{.5f};
        float c_balance_{0.f}, c_separation_{0.f}, c_hold_{0.f}, c_smooth_{0.f};

        void setOrder(const size_t, const size_t order) override {
            zldsp::filter::FIRBase<FloatType, 10>::setFFTOrder(1, order);
            // set fft data lines
            fft_line_pos_ = 0;
            for (auto &line: fft_lines_) {
                for (auto& plane : line) {
                    plane.assign(this->num_bins_, 0.f);
                }
            }
            magnitude_.resize(this->num_bins_);
            time_median_.resize(this->num_bins_);
            for (auto& median : time_median_) { median.clear(); }
            freq_median_.clear();
            mask_.assign(this->num_bins_, 0.f);
        }

        void processSpectrum() override {
            // get real/imaginary output planes
            namespace hn = hwy::HWY_NAMESPACE;
            const hn::ScalableTag<float> d;
            const auto lanes = hn::Lanes(d);
            size_t bin = 0;
            for (; bin + lanes <= this->num_bins_; bin += lanes) {
                const auto re = hn::LoadU(d, this->fft_data_[0].data() + bin);
                const auto im = hn::LoadU(d, this->fft_data_[1].data() + bin);
                hn::StoreU(hn::Sqrt(hn::Add(hn::Mul(re, re), hn::Mul(im, im))), d, magnitude_.data() + bin);
            }
            for (; bin < this->num_bins_; ++bin) {
                const auto re = this->fft_data_[0][bin];
                const auto im = this->fft_data_[1][bin];
                magnitude_[bin] = std::sqrt(re * re + im * im);
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
            const auto update_mask = [this](const size_t i, const float next_magnitude) {
                freq_median_.insert(next_magnitude);
                time_median_[i].insert(magnitude_[i]);
                const auto c_mask = calculatePortion(freq_median_.getMedian(), time_median_[i].getMedian());
                mask_[i] = std::max(mask_[i] * c_hold_, c_mask);
            };
            const auto interior_end = this->num_bins_ - kFreqHalfMedianWindowsSize;
            for (size_t i = 0; i < interior_end; ++i) {
                update_mask(i, magnitude_[i + kFreqHalfMedianWindowsSize]);
            }
            // extend Nyquist for median calculation
            for (size_t i = interior_end; i < this->num_bins_; ++i) {
                update_mask(i, magnitude_.back());
            }
            // swap with the frame from two hops ago
            this->fft_data_.swap(fft_lines_[fft_line_pos_]);
            fft_line_pos_ = (fft_line_pos_ + 1) % kTimeHalfMedianWindowsSize;
            auto mask_mean = zldsp::vector::sum(mask_.data(), mask_.size()) / static_cast<float>(mask_.size());
            mask_mean = std::clamp((mask_mean - 0.5f) * std::sqrt(c_separation_), -.5f, .5f) + .5f;
            // smooth the mask using magnitude scratch
            zldsp::vector::fma(magnitude_.data(), mask_.data(), 1.f - c_smooth_, mask_mean * c_smooth_, this->num_bins_);
            for (auto& plane : this->fft_data_) {
                zldsp::vector::multiply(plane.data(), magnitude_.data(), this->num_bins_);
            }
        }

        float calculatePortion(const float transient_weight, const float steady_weight) const {
            const auto t = transient_weight * c_balance_;
            const auto s = steady_weight;
            const auto tt = t * t;
            const auto ss = s * s;
            const auto p = tt / std::max(tt + ss, 0.00000001f);
            return std::clamp((p - 0.5f) * c_separation_, -.5f, .5f) + .5f;
        }
    };
}