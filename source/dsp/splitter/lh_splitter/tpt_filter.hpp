// Copyright (C) 2026 - zsliu98
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
    class TPTFilter {
    public:
        enum TPTFilterType {
            kLowPass, kHighPass, kAllPass
        };

        TPTFilter() = default;

        void reset() {
            std::fill(s1_.begin(), s1_.end(), 0.0);
            std::fill(s2_.begin(), s2_.end(), 0.0);
        }

        template<bool Update = true>
        void setFreq(const double freq) {
            freq_ = freq;
            if constexpr (Update) {
                updateCoeff();
            }
        }

        template<bool Update = true>
        void setQ(const double q) {
            q_ = q;
            if constexpr (Update) {
                updateCoeff();
            }
        }

        void updateCoeff() {
            g_ = std::tan(sample_rate_scale_ * freq_);
            g2_ = g_ * 2.0;
            R2_ = 1.0 / q_;
            g_R2_ = g_ + R2_;
            h_ = 1.0 / (1.0 + R2_ * g_ + g_ * g_);
        }

        void prepare(const double sample_rate, const size_t num_channels) {
            sample_rate_scale_ = std::numbers::pi / sample_rate;
            s1_.resize(num_channels);
            s2_.resize(num_channels);
            reset();
            updateCoeff();
        }

        template<TPTFilterType FilterType>
        FloatType processSample(const size_t chan, FloatType x) {
            if constexpr (FilterType == kLowPass) {
                const auto y_bp = (g_ * (static_cast<double>(x) - s2_[chan]) + s1_[chan]) * h_;
                const auto v1 = y_bp - s1_[chan];
                s1_[chan] = y_bp + v1;
                const auto v2 = g_ * y_bp;
                const auto y_lp = v2 + s2_[chan];
                s2_[chan] = y_lp + v2;
                return static_cast<FloatType>(y_lp);
            }
            if constexpr (FilterType == kHighPass) {
                const auto y_hp = h_ * (static_cast<double>(x) - s1_[chan] * g_R2_ - s2_[chan]);
                const auto y_hp_g = y_hp * g_;
                const auto y_bp = y_hp_g + s1_[chan];
                s1_[chan] = y_hp_g + y_bp;
                s2_[chan] = y_bp * g2_ + s2_[chan];
                return static_cast<FloatType>(y_hp);
            }
            if constexpr (FilterType == kAllPass) {
                const auto y_hp = h_ * (static_cast<double>(x) - s1_[chan] * g_R2_ - s2_[chan]);
                const auto y_hp_g = y_hp * g_;
                const auto y_bp = y_hp_g + s1_[chan];
                s1_[chan] = y_hp_g + y_bp;
                const auto y_bp_g = y_bp * g_;
                const auto y_lp = y_bp_g + s2_[chan];
                s2_[chan] = y_bp_g + y_lp;
                return static_cast<FloatType>(y_lp - R2_ * y_bp + y_hp);
            }
            return static_cast<FloatType>(0);
        }

        void processSampleLowHigh(const size_t chan, FloatType x,
                                  FloatType &low, FloatType &high) {
            const auto y_hp = h_ * (static_cast<double>(x) - s1_[chan] * g_R2_ - s2_[chan]);
            const auto y_bp = y_hp * g_ + s1_[chan];
            s1_[chan] = y_hp * g_ + y_bp;
            const auto y_lp = y_bp * g_ + s2_[chan];
            s2_[chan] = y_bp * g_ + y_lp;
            low = static_cast<FloatType>(y_lp);
            high = static_cast<FloatType>(y_hp);
        }

    private:
        double sample_rate_scale_{1.0};
        double freq_{1000.0}, q_{0.707};
        double g_{}, g2_{}, h_{}, R2_{}, g_R2_{};
        std::vector<double> s1_, s2_;
    };
}