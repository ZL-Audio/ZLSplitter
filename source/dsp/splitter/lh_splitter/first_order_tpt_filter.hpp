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
#include <span>
#include <numbers>

namespace zldsp::splitter {
    /**
     * A second order TPT Filter which simultaneously produces lowpass, bandpass, highpass.
     * It is suitable for fast parameter modulation.
     * @tparam FloatType
     */
    template<typename FloatType>
    class FirstOrderTPTFilter {
    public:
        enum TPTFilterType {
            kLowPass, kHighPass, kAllPass
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

        void prepare(const double sample_rate, const size_t num_channels) {
            sample_rate_scale_ = std::numbers::pi / sample_rate;
            s_.resize(num_channels);
            reset();
            updateCoeff();
        }

        template<TPTFilterType FilterType>
        FloatType processSample(const size_t chan, FloatType x) {
            const auto v = (static_cast<double>(x) - s_[chan]) * g_;
            const auto y_lp = v + s_[chan];
            s_[chan] = y_lp + v;
            switch (FilterType) {
                case kLowPass: {
                    return static_cast<FloatType>(y_lp);
                }
                case kHighPass: {
                    return static_cast<FloatType>(y_lp) - x;
                }
                case kAllPass: {
                    return static_cast<FloatType>(y_lp + y_lp) - x;
                }
                default: {
                    return static_cast<FloatType>(0);
                }
            }
        }

        void processSampleLowHigh(const size_t chan, FloatType x,
                                  FloatType &low, FloatType &high) {
            const auto v = (static_cast<double>(x) - s_[chan]) * g_;
            const auto y_lp = v + s_[chan];
            s_[chan] = y_lp + v;
            low = static_cast<FloatType>(y_lp);
            high = x - low;
        }

    private:
        double sample_rate_scale_{1.0};
        double freq_{1000.0};
        double g_{};
        std::vector<double> s_;
    };
}
