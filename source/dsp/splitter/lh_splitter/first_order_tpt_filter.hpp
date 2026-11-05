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
#include <cmath>
#include <numbers>
#include <vector>

namespace zldsp::splitter {
    /**
     * A first order TPT Filter which simultaneously produces lowpass and highpass.
     * It is suitable for fast parameter modulation.
     * @tparam FloatType
     */
    template<typename FloatType>
    class FirstOrderTPTFilter {
    public:
        enum TPTFilterType {
            kLowPass, kHighPass, kAllPass
        };

        struct LocalState {
            double g{}, s{};

            template<TPTFilterType FilterType>
            FloatType processSample(FloatType x) {
                const auto v = (static_cast<double>(x) - s) * g;
                const auto y_lp = v + s;
                s = y_lp + v;
                if constexpr (FilterType == kLowPass) {
                    return static_cast<FloatType>(y_lp);
                }
                if constexpr (FilterType == kHighPass) {
                    return static_cast<FloatType>(y_lp) - x;
                }
                if constexpr (FilterType == kAllPass) {
                    return static_cast<FloatType>(y_lp + y_lp) - x;
                }
                return static_cast<FloatType>(0);
            }

            void processSampleLowHigh(FloatType x, FloatType &low, FloatType &high) {
                const auto v = (static_cast<double>(x) - s) * g;
                const auto y_lp = v + s;
                s = y_lp + v;
                low = static_cast<FloatType>(y_lp);
                high = x - low;
            }
        };

        FirstOrderTPTFilter() = default;

        void reset() {
            std::fill(s_.begin(), s_.end(), 0.0);
        }

        template<bool Update = true>
        void setFreq(const double freq) {
            freq_ = freq;
            if constexpr (Update) {
                updateCoeff();
            }
        }

        void updateCoeff() {
            const auto t = std::tan(sample_rate_scale_ * freq_);
            g_ = t / (1.0 + t);
        }

        /**
         * copy coefficients from a filter prepared at the same sample rate
         * @param source
         */
        void copyCoefficientsFrom(const FirstOrderTPTFilter &source) {
            freq_ = source.freq_;
            g_ = source.g_;
        }

        void prepare(const double sample_rate, const size_t num_channels) {
            sample_rate_scale_ = std::numbers::pi / sample_rate;
            s_.resize(num_channels);
            reset();
            updateCoeff();
        }

        template<TPTFilterType FilterType>
        FloatType processSample(const size_t chan, FloatType x) {
            auto state = getLocalState(chan);
            const auto y = state.template processSample<FilterType>(x);
            saveLocalState(chan, state);
            return y;
        }

        void processSampleLowHigh(const size_t chan, FloatType x,
                                  FloatType &low, FloatType &high) {
            auto state = getLocalState(chan);
            state.processSampleLowHigh(x, low, high);
            saveLocalState(chan, state);
        }

        LocalState getLocalState(const size_t chan) const {
            return {g_, s_[chan]};
        }

        void saveLocalState(const size_t chan, const LocalState &state) {
            s_[chan] = state.s;
        }

    private:
        double sample_rate_scale_{1.0};
        double freq_{1000.0};
        double g_{};
        std::vector<double> s_;
    };
}
