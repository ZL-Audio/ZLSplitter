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
#include <vector>
#include <span>

namespace zldsp::filter {
    template<typename FloatType>
    class IIRBase {
    public:
        struct LocalState {
            std::array<FloatType, 5> coeff{};
            FloatType s1{}, s2{};

            FloatType processSample(const FloatType input) {
                return IIRBase::processSample(coeff, s1, s2, input);
            }
        };

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

        LocalState getLocalState(const size_t channel) const {
            return {coeff_, s1_[channel], s2_[channel]};
        }

        void saveLocalState(const size_t channel, const LocalState &state) {
            s1_[channel] = state.s1;
            s2_[channel] = state.s2;
        }

        // Normalized ZLEQ layout: {a1, a2, b0, b1, b2}, with a0 = 1.
        void updateFromBiquad(const std::array<double, 5> &coeff) {
            std::copy(coeff.begin(), coeff.end(), coeff_.begin());
        }

    private:
        std::array<FloatType, 5> coeff_{0, 0, 0, 0, 0};
        std::vector<FloatType> s1_, s2_;

        static FloatType processSample(const std::array<FloatType, 5> &coeff,
                                       FloatType &s1, FloatType &s2, FloatType inputValue) {
            const auto outputValue = inputValue * coeff[2] + s1;
            s1 = (inputValue * coeff[3]) - (outputValue * coeff[0]) + s2;
            s2 = (inputValue * coeff[4]) - (outputValue * coeff[1]);
            return outputValue;
        }
    };
}
