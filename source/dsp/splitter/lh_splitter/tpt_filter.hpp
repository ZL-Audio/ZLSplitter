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

        struct LocalState {
            double g{}, g2{}, h{}, R2{}, g_R2{}, s1{}, s2{};

            template<TPTFilterType FilterType>
            FloatType processSample(FloatType x) {
                if constexpr (FilterType == kLowPass) {
                    const auto y_bp = (g * (static_cast<double>(x) - s2) + s1) * h;
                    const auto v1 = y_bp - s1;
                    s1 = y_bp + v1;
                    const auto v2 = g * y_bp;
                    const auto y_lp = v2 + s2;
                    s2 = y_lp + v2;
                    return static_cast<FloatType>(y_lp);
                }
                if constexpr (FilterType == kHighPass) {
                    const auto y_hp = h * (static_cast<double>(x) - s1 * g_R2 - s2);
                    const auto y_hp_g = y_hp * g;
                    const auto y_bp = y_hp_g + s1;
                    s1 = y_hp_g + y_bp;
                    s2 = y_bp * g2 + s2;
                    return static_cast<FloatType>(y_hp);
                }
                if constexpr (FilterType == kAllPass) {
                    const auto y_hp = h * (static_cast<double>(x) - s1 * g_R2 - s2);
                    const auto y_hp_g = y_hp * g;
                    const auto y_bp = y_hp_g + s1;
                    s1 = y_hp_g + y_bp;
                    const auto y_bp_g = y_bp * g;
                    const auto y_lp = y_bp_g + s2;
                    s2 = y_bp_g + y_lp;
                    return static_cast<FloatType>(y_lp - R2 * y_bp + y_hp);
                }
                return static_cast<FloatType>(0);
            }

            void processSampleLowHigh(FloatType x, FloatType &low, FloatType &high) {
                const auto y_hp = h * (static_cast<double>(x) - s1 * g_R2 - s2);
                const auto y_bp = y_hp * g + s1;
                s1 = y_hp * g + y_bp;
                const auto y_lp = y_bp * g + s2;
                s2 = y_bp * g + y_lp;
                low = static_cast<FloatType>(y_lp);
                high = static_cast<FloatType>(y_hp);
            }
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
            updateCoeff(std::tan(sample_rate_scale_ * freq_));
        }

        double getWarpedFreq() const {
            return g_;
        }

        /**
         * set the frequency using a precomputed tan(pi * freq / sample_rate)
         * @param freq
         * @param g
         */
        void setWarpedFreq(const double freq, const double g) {
            freq_ = freq;
            updateCoeff(g);
        }

        /**
         * copy coefficients from a filter prepared at the same sample rate
         * @param source
         */
        void copyCoefficientsFrom(const TPTFilter &source) {
            freq_ = source.freq_;
            q_ = source.q_;
            g_ = source.g_;
            g2_ = source.g2_;
            h_ = source.h_;
            R2_ = source.R2_;
            g_R2_ = source.g_R2_;
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
            return {g_, g2_, h_, R2_, g_R2_, s1_[chan], s2_[chan]};
        }

        void saveLocalState(const size_t chan, const LocalState &state) {
            s1_[chan] = state.s1;
            s2_[chan] = state.s2;
        }

    private:
        double sample_rate_scale_{1.0};
        double freq_{1000.0}, q_{0.707};
        double g_{}, g2_{}, h_{}, R2_{}, g_R2_{};
        std::vector<double> s1_, s2_;

        void updateCoeff(const double g) {
            g_ = g;
            g2_ = g_ * 2.0;
            R2_ = 1.0 / q_;
            g_R2_ = g_ + R2_;
            h_ = 1.0 / (1.0 + R2_ * g_ + g_ * g_);
        }
    };
}
