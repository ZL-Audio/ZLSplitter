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
#include <numbers>
#include <vector>
#include <complex>
#include <span>

namespace zldsp::filter {
    template<typename FloatType>
    class IIRBase {
    public:
        // w should be an array of std::exp(-2pi * f / samplerate * i)
        static void updateResponse(
            const std::array<double, 6> &coeff,
            const std::vector<std::complex<FloatType>> &wis, std::vector<std::complex<FloatType>> &response) {
            for (size_t idx = 0; idx < wis.size(); ++idx) {
                response[idx] *= getResponse(coeff, wis[idx]);
            }
        }

        // w should be std::exp(-2pi * f / samplerate * i)
        // const auto wi = std::exp(std::complex<FloatType>(FloatType(0), w))
        static std::complex<FloatType> getResponse(const std::array<double, 6> &coeff,
                                                   const std::complex<FloatType> &wi) {
            const auto wi2 = wi * wi;
            return (static_cast<FloatType>(coeff[3]) +
                    static_cast<FloatType>(coeff[4]) * wi +
                    static_cast<FloatType>(coeff[5]) * wi2) / (
                       static_cast<FloatType>(coeff[0]) +
                       static_cast<FloatType>(coeff[1]) * wi +
                       static_cast<FloatType>(coeff[2]) * wi2);
        }

        IIRBase() = default;

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
            if (num_samples == 0) {
                return;
            }
            const auto coeff = coeff_;
            for (size_t channel = 0; channel < buffer.size(); ++channel) {
                auto *samples = buffer[channel];
                auto s1 = s1_[channel], s2 = s2_[channel];
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto output = processSample(coeff, s1, s2, samples[i]);
                    if constexpr (!isBypass) {
                        samples[i] = output;
                    }
                }
                s1_[channel] = s1;
                s2_[channel] = s2;
            }
        }

        FloatType processSample(const size_t channel, FloatType inputValue) {
            return processSample(coeff_, s1_[channel], s2_[channel], inputValue);
        }

        void updateFromBiquad(const std::array<double, 6> &coeff) {
            const auto a0_inv = 1.0 / coeff[0];
            coeff_[0] = static_cast<FloatType>(coeff[3] * a0_inv);
            coeff_[1] = static_cast<FloatType>(coeff[4] * a0_inv);
            coeff_[2] = static_cast<FloatType>(coeff[5] * a0_inv);
            coeff_[3] = static_cast<FloatType>(coeff[1] * a0_inv);
            coeff_[4] = static_cast<FloatType>(coeff[2] * a0_inv);
        }

    private:
        std::array<FloatType, 5> coeff_{0, 0, 0, 0, 0};
        std::vector<FloatType> s1_, s2_;

        static FloatType processSample(const std::array<FloatType, 5> &coeff,
                                       FloatType &s1, FloatType &s2, FloatType inputValue) {
            const auto outputValue = inputValue * coeff[0] + s1;
            s1 = (inputValue * coeff[1]) - (outputValue * coeff[3]) + s2;
            s2 = (inputValue * coeff[2]) - (outputValue * coeff[4]);
            return outputValue;
        }
    };
}
