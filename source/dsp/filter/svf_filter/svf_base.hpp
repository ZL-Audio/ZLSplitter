// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <span>

namespace zldsp::filter {
    template<typename FloatType>
    class SVFBase {
    public:
        SVFBase() = default;

        void prepare(const size_t num_channels) {
            s1_.resize(num_channels);
            s2_.resize(num_channels);
            reset();
        }

        void reset() {
            std::fill(s1_.begin(), s1_.end(), static_cast<FloatType>(0));
            std::fill(s2_.begin(), s2_.end(), static_cast<FloatType>(0));
        }

        template<bool isBypass = false>
        void process(std::span<FloatType *> buffer, const size_t num_samples) noexcept {
            if constexpr (isBypass) {
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    auto *samples = buffer[channel];
                    for (size_t i = 0; i < num_samples; ++i) {
                        processSample(channel, samples[i]);
                    }
                }
            } else {
                for (size_t channel = 0; channel < buffer.size(); ++channel) {
                    auto *samples = buffer[channel];
                    for (size_t i = 0; i < num_samples; ++i) {
                        samples[i] = processSample(channel, samples[i]);
                    }
                }
            }
        }

        FloatType processSample(const size_t channel, FloatType input_value) {
            const auto y_hp = h_ * (input_value - s1_[channel] * (g_ + R2_) - s2_[channel]);

            const auto y_bp = y_hp * g_ + s1_[channel];
            s1_[channel] = y_hp * g_ + y_bp;

            const auto y_lp = y_bp * g_ + s2_[channel];
            s2_[channel] = y_bp * g_ + y_lp;

            return chp_ * y_hp + cbp_ * y_bp + clp_ * y_lp;
        }

        void updateFromBiquad(const std::array<double, 6> &coeffs) {
            const auto temp1 = std::sqrt(std::abs((-coeffs[0] - coeffs[1] - coeffs[2])));
            const auto temp2 = std::sqrt(std::abs((-coeffs[0] + coeffs[1] - coeffs[2])));
            g_ = static_cast<FloatType>(temp1 / temp2);
            R2_ = static_cast<FloatType>(2 * (coeffs[0] - coeffs[2]) / (temp1 * temp2));
            h_ = static_cast<FloatType>(1) / (g_ * (R2_ + g_) + static_cast<FloatType>(1));

            chp_ = static_cast<FloatType>((coeffs[3] - coeffs[4] + coeffs[5]) / (coeffs[0] - coeffs[1] + coeffs[2]));
            cbp_ = static_cast<FloatType>(2 * (coeffs[5] - coeffs[3]) / (temp1 * temp2));
            clp_ = static_cast<FloatType>((coeffs[3] + coeffs[4] + coeffs[5]) / (coeffs[0] + coeffs[1] + coeffs[2]));
        }

    private:
        FloatType g_, R2_, h_, chp_, cbp_, clp_;
        std::vector<FloatType> s1_, s2_;
    };
}