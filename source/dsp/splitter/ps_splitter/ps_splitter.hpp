// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <atomic>

#include "../../container/circular_buffer.hpp"

namespace zldsp::splitter {
    /**
     * a splitter that splits the stereo audio signal input peak signal and steady signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class PSSplitter {
    public:
        PSSplitter() = default;

        void prepare(const double sample_rate) {
            sample_rate_ = static_cast<FloatType>(sample_rate);
            peak_sm_buffer_.setCapacity(static_cast<size_t>(sample_rate * .01));
            peak_sm_buffer_.clear();
            steady_sm_buffer_.setCapacity(static_cast<size_t>(sample_rate * 1.));
            steady_sm_buffer_.clear();
            to_update_.store(true, std::memory_order::release);
        }

        void prepareBuffer() {
            if (to_update_.exchange(false, std::memory_order::acquire)) {
                updatePara();
            }
        }

        void process(FloatType * in_buffer,
                     FloatType * peak_buffer,
                     FloatType * steady_buffer,
                     const size_t num_samples) {
            while (peak_sm_buffer_.size() >= peak_buffer_size_) {
                peak_sm_ -= peak_sm_buffer_.popFront();
            }
            while (steady_sm_buffer_.size() >= steady_buffer_size_) {
                steady_sm_ -= steady_sm_buffer_.popFront();
            }
            for (size_t i = 0; i < num_samples; ++i) {
                while (peak_sm_buffer_.size() >= peak_buffer_size_) {
                    peak_sm_ -= peak_sm_buffer_.popFront();
                }
                while (steady_sm_buffer_.size() >= steady_buffer_size_) {
                    steady_sm_ -= steady_sm_buffer_.popFront();
                }
                const auto sample = in_buffer[i];
                const auto square = sample * sample;
                peak_sm_buffer_.pushBack(square);
                steady_sm_buffer_.pushBack(square);
                peak_sm_ += square;
                steady_sm_ += square;

                const auto threshold = steady_sm_ * steady_buffer_size_inverse_ * c_balance_;
                if (peak_sm_ * peak_buffer_size_inverse_ > threshold) {
                    mask_ = mask_ * c_attack_ + c_attack_c_;
                } else {
                    mask_ *= c_release_;
                }

                const auto peak = sample * mask_;
                peak_buffer[i] = peak;
                steady_buffer[i] = sample - peak;
            }
        }

        void setBalance(const FloatType x) {
            balance_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setAttack(const FloatType x) {
            attack_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setHold(const FloatType x) {
            hold_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setSmooth(const FloatType x) {
            smooth_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

    private:
        std::atomic<FloatType> attack_{FloatType(0.5)}, balance_{FloatType(0.5)}, hold_{FloatType(0.5)}, smooth_{FloatType(0.5)};
        FloatType sample_rate_{FloatType(48000)};
        std::atomic<bool> to_update_{true};
        FloatType c_balance_{}, c_release_{}, c_attack_{}, c_attack_c_{};
        FloatType peak_buffer_size_inverse_{1}, steady_buffer_size_inverse_{1};
        size_t peak_buffer_size_{1}, steady_buffer_size_{1};
        FloatType peak_sm_{0}, steady_sm_{0}, mask_{0};
        zldsp::container::CircularBuffer<FloatType> peak_sm_buffer_{1}, steady_sm_buffer_{1};

        void updatePara() {
            c_balance_ = std::pow(FloatType(10), FloatType(1) - balance_.load());
            c_balance_ = c_balance_ * c_balance_;
            c_release_ = std::pow(FloatType(0.9) * cube(hold_.load()) + FloatType(5e-2), FloatType(10) / sample_rate_);
            c_attack_ = std::pow(FloatType(1e-4), (FloatType(500) - FloatType(450) * attack_.load()) / sample_rate_);
            c_attack_c_ = FloatType(1) - c_attack_;
            const auto c_smooth = std::max(smooth_.load(), FloatType(0.01));
            peak_buffer_size_ = static_cast<size_t>(c_smooth * static_cast<FloatType>(peak_sm_buffer_.capacity()));
            peak_buffer_size_inverse_ = FloatType(1) / static_cast<FloatType>(peak_buffer_size_);
            steady_buffer_size_ = static_cast<size_t>(c_smooth * static_cast<FloatType>(steady_sm_buffer_.capacity()));
            steady_buffer_size_ = std::max(steady_buffer_size_, peak_buffer_size_);
            steady_buffer_size_inverse_ = FloatType(1) / static_cast<FloatType>(steady_buffer_size_);
        }

        static constexpr FloatType cube(const FloatType x) {
            return x * x * x;
        }
    };
}
