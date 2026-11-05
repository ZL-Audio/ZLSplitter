// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include "../../chore/smoothed_value.hpp"
#include "../../vector/highway_import.hpp"

namespace zldsp::splitter {
    template<typename FloatType>
    class MSSplitter {
    public:
        MSSplitter() = default;

        void prepare(const double sample_rate) {
            c_mix_.prepare(sample_rate, 0.1);
        }

        void setMix(const FloatType mix) {
            c_mix_.setTarget(static_cast<FloatType>(0.5) - mix);
        }

        HWY_ATTR void process(std::array<FloatType *, 2> &in_buffer,
                              std::array<FloatType *, 2> &mid_buffer,
                              std::array<FloatType *, 2> &side_buffer,
                              const size_t num_samples) {
            const auto in_l = in_buffer[0], in_r = in_buffer[1];
            const auto mid_l = mid_buffer[0], mid_r = mid_buffer[1];
            const auto side_l = side_buffer[0], side_r = side_buffer[1];
            if (c_mix_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto c_mix = c_mix_.getNext();
                    const auto l = in_l[i], r = in_r[i];
                    const auto l_mix = static_cast<FloatType>(0.5) * l - c_mix * r;
                    const auto r_mix = static_cast<FloatType>(0.5) * r - c_mix * l;
                    side_l[i] = l_mix;
                    side_r[i] = r_mix;
                    mid_l[i] = l - l_mix;
                    mid_r[i] = r - r_mix;
                }
            } else {
                namespace hn = hwy::HWY_NAMESPACE;
                static constexpr hn::ScalableTag<FloatType> d;
                static constexpr auto lanes = hn::MaxLanes(d);
                const auto vector_samples = num_samples - num_samples % lanes;
                const auto c_mix = c_mix_.getCurrent();
                const auto v_half = hn::Set(d, static_cast<FloatType>(0.5));
                const auto v_c_mix = hn::Set(d, c_mix);
                size_t i = 0;
                for (; i < vector_samples; i += lanes) {
                    const auto l = hn::LoadU(d, in_l + i);
                    const auto r = hn::LoadU(d, in_r + i);
                    const auto l_mix = hn::Sub(hn::Mul(v_half, l), hn::Mul(v_c_mix, r));
                    const auto r_mix = hn::Sub(hn::Mul(v_half, r), hn::Mul(v_c_mix, l));
                    hn::StoreU(l_mix, d, side_l + i);
                    hn::StoreU(r_mix, d, side_r + i);
                    hn::StoreU(hn::Sub(l, l_mix), d, mid_l + i);
                    hn::StoreU(hn::Sub(r, r_mix), d, mid_r + i);
                }
                for (; i < num_samples; ++i) {
                    const auto l = in_l[i], r = in_r[i];
                    const auto l_mix = static_cast<FloatType>(0.5) * l - c_mix * r;
                    const auto r_mix = static_cast<FloatType>(0.5) * r - c_mix * l;
                    side_l[i] = l_mix;
                    side_r[i] = r_mix;
                    mid_l[i] = l - l_mix;
                    mid_r[i] = r - r_mix;
                }
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> c_mix_{static_cast<FloatType>(0.5)};
    };
}
