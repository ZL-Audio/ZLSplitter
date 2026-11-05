// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>

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
        static_assert(kFreqMedianWindowsSize == 5 && kTimeMedianWindowsSize == 5);

        std::array<FloatType *, 1> delay_span_;
        zldsp::delay::IntegerDelay<FloatType> delay_;
        // extra fft working space
        std::array<FloatType *, 1> fft_span_;
        size_t fft_line_pos_ = 0;
        std::array<typename zldsp::filter::FIRBase<FloatType, 10>::Spectrum, kTimeHalfMedianWindowsSize> fft_lines_;
        // magnitude history
        std::array<zldsp::vector::aligned_vector<float>, kTimeMedianWindowsSize> magnitude_history_;
        size_t magnitude_pos_{0}, magnitude_count_{0};
        // portion holders
        zldsp::vector::aligned_vector<float> mask_, smoothed_mask_;
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
            for (auto& frame : magnitude_history_) {
                frame.assign(this->num_bins_, 0.f);
            }
            magnitude_pos_ = magnitude_count_ = 0;
            mask_.assign(this->num_bins_, 0.f);
            smoothed_mask_.resize(this->num_bins_);
        }

        void processSpectrum() override {
            // get real/imaginary output planes
            namespace hn = hwy::HWY_NAMESPACE;
            static constexpr hn::ScalableTag<float> d;
            static constexpr size_t lanes = hn::MaxLanes(d);
            auto* magnitude = magnitude_history_[magnitude_pos_].data();
            size_t bin = 0;
            for (; bin + lanes <= this->num_bins_; bin += lanes) {
                const auto re = hn::LoadU(d, this->fft_data_[0].data() + bin);
                const auto im = hn::LoadU(d, this->fft_data_[1].data() + bin);
                hn::StoreU(hn::Sqrt(hn::Add(hn::Mul(re, re), hn::Mul(im, im))), d, magnitude + bin);
            }
            for (; bin < this->num_bins_; ++bin) {
                const auto re = this->fft_data_[0][bin];
                const auto im = this->fft_data_[1][bin];
                magnitude[bin] = std::sqrt(re * re + im * im);
            }
            // load parameters
            c_balance_ = balance_.load(std::memory_order::relaxed);
            c_separation_ = separation_.load(std::memory_order::relaxed);
            c_hold_ = hold_.load(std::memory_order::relaxed);
            c_smooth_ = smooth_.load(std::memory_order::relaxed);
            // calculate mask
            magnitude_count_ = std::min(magnitude_count_ + 1, kTimeMedianWindowsSize);
            updateMask(magnitude);
            magnitude_pos_ = (magnitude_pos_ + 1) % kTimeMedianWindowsSize;
            // swap with the frame from two hops ago
            this->fft_data_.swap(fft_lines_[fft_line_pos_]);
            fft_line_pos_ = (fft_line_pos_ + 1) % kTimeHalfMedianWindowsSize;
            auto mask_mean = zldsp::vector::sum(mask_.data(), mask_.size()) / static_cast<float>(mask_.size());
            mask_mean = std::clamp((mask_mean - 0.5f) * std::sqrt(c_separation_), -.5f, .5f) + .5f;
            // smooth the mask
            zldsp::vector::fma(smoothed_mask_.data(), mask_.data(), 1.f - c_smooth_, mask_mean * c_smooth_, this->num_bins_);
            for (auto& plane : this->fft_data_) {
                zldsp::vector::multiply(plane.data(), smoothed_mask_.data(), this->num_bins_);
            }
        }

        static void sortPair(float& a, float& b) {
            const auto lo = std::min(a, b);
            b = std::max(a, b);
            a = lo;
        }

        template<typename V>
        static void sortPair(V& a, V& b) {
            namespace hn = hwy::HWY_NAMESPACE;
            const auto lo = hn::Min(a, b);
            b = hn::Max(a, b);
            a = lo;
        }

        /**
         * calculate the median of five scalar or SIMD values using a comparison network
         */
        template<typename Value>
        static Value calculateMedian5(Value a, Value b, Value c, Value d, Value e) {
            sortPair(a, b);
            sortPair(d, e);
            sortPair(c, e);
            sortPair(c, d);
            sortPair(b, e);
            sortPair(a, d);
            sortPair(a, c);
            sortPair(b, d);
            sortPair(b, c);
            return c;
        }

        float calculateTimeMedian(const size_t i) const {
            auto a = magnitude_history_[0][i];
            if (magnitude_count_ == 1) {
                return a;
            }
            auto b = magnitude_history_[1][i];
            if (magnitude_count_ == 2) {
                return (a + b) / 2.f;
            }
            auto c = magnitude_history_[2][i];
            if (magnitude_count_ == 3) {
                sortPair(a, b);
                sortPair(b, c);
                sortPair(a, b);
                return b;
            }
            auto d = magnitude_history_[3][i];
            if (magnitude_count_ == 4) {
                sortPair(a, b);
                sortPair(c, d);
                sortPair(a, c);
                sortPair(b, d);
                sortPair(b, c);
                return (b + c) / 2.f;
            }
            return calculateMedian5(a, b, c, d, magnitude_history_[4][i]);
        }

        void updateMask(const float* magnitude) {
            namespace hn = hwy::HWY_NAMESPACE;
            static constexpr hn::ScalableTag<float> d;
            static constexpr size_t lanes = hn::MaxLanes(d);
            const auto last_bin = this->num_bins_ - 1;
            const auto update_mask = [this, magnitude, last_bin](const size_t i) {
                const auto transient_weight = calculateMedian5(
                    magnitude[i >= 2 ? i - 2 : 0], magnitude[i >= 1 ? i - 1 : 0], magnitude[i],
                    magnitude[std::min(i + 1, last_bin)], magnitude[std::min(i + 2, last_bin)]);
                const auto c_mask = calculatePortion(transient_weight, calculateTimeMedian(i));
                mask_[i] = std::max(mask_[i] * c_hold_, c_mask);
            };
            size_t i = 0;
            if (magnitude_count_ == kTimeMedianWindowsSize) {
                // extend DC for median calculation
                for (; i < kFreqHalfMedianWindowsSize; ++i) {
                    update_mask(i);
                }
                const auto balance = hn::Set(d, c_balance_);
                const auto separation = hn::Set(d, c_separation_);
                const auto hold = hn::Set(d, c_hold_);
                const auto half = hn::Set(d, .5f);
                const auto neg_half = hn::Set(d, -.5f);
                const auto epsilon = hn::Set(d, .00000001f);
                const auto interior_end = this->num_bins_ - kFreqHalfMedianWindowsSize;
                for (; i + lanes <= interior_end; i += lanes) {
                    const auto transient_weight = calculateMedian5(
                        hn::LoadU(d, magnitude + i - 2), hn::LoadU(d, magnitude + i - 1),
                        hn::LoadU(d, magnitude + i), hn::LoadU(d, magnitude + i + 1),
                        hn::LoadU(d, magnitude + i + 2));
                    const auto steady_weight = calculateMedian5(
                        hn::LoadU(d, magnitude_history_[0].data() + i), hn::LoadU(d, magnitude_history_[1].data() + i),
                        hn::LoadU(d, magnitude_history_[2].data() + i), hn::LoadU(d, magnitude_history_[3].data() + i),
                        hn::LoadU(d, magnitude_history_[4].data() + i));
                    const auto t = hn::Mul(transient_weight, balance);
                    const auto tt = hn::Mul(t, t);
                    const auto ss = hn::Mul(steady_weight, steady_weight);
                    const auto p = hn::Div(tt, hn::Max(hn::Add(tt, ss), epsilon));
                    const auto c_mask = hn::Add(hn::Min(hn::Max(hn::Mul(hn::Sub(p, half), separation), neg_half), half), half);
                    hn::StoreU(hn::Max(hn::Mul(hn::LoadU(d, mask_.data() + i), hold), c_mask), d, mask_.data() + i);
                }
            }
            // handle startup frames, remaining bins, and Nyquist extension
            for (; i < this->num_bins_; ++i) {
                update_mask(i);
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
