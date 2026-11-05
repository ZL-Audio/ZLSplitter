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
    class LRSplitter {
    public:
        LRSplitter() = default;

        void prepare(const double sample_rate) {
            mix_.prepare(sample_rate, 0.1);
        }

        void setMix(const FloatType mix) {
            mix_.setTarget(mix);
        }

        HWY_ATTR void process(std::array<FloatType *, 2> &in_buffer,
                              std::array<FloatType *, 2> &left_buffer,
                              std::array<FloatType *, 2> &right_buffer,
                              const size_t num_samples) {
            const auto in_l = in_buffer[0], in_r = in_buffer[1];
            const auto left_l = left_buffer[0], left_r = left_buffer[1];
            const auto right_l = right_buffer[0], right_r = right_buffer[1];
            if (mix_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto mix = mix_.getNext();
                    const auto l = in_l[i], r = in_r[i];
                    const auto l_mix = l * mix, r_mix = r * (static_cast<FloatType>(1.0) - mix);
                    right_l[i] = l_mix;
                    right_r[i] = r_mix;
                    left_l[i] = l - l_mix;
                    left_r[i] = r - r_mix;
                }
            } else {
                namespace hn = hwy::HWY_NAMESPACE;
                static constexpr hn::ScalableTag<FloatType> d;
                static constexpr auto lanes = hn::MaxLanes(d);
                const auto vector_samples = num_samples - num_samples % lanes;
                const auto mix = mix_.getCurrent();
                const auto c_mix = static_cast<FloatType>(1.0) - mix;
                const auto v_mix = hn::Set(d, mix);
                const auto v_c_mix = hn::Set(d, c_mix);
                size_t i = 0;
                for (; i < vector_samples; i += lanes) {
                    const auto l = hn::LoadU(d, in_l + i);
                    const auto r = hn::LoadU(d, in_r + i);
                    const auto l_mix = hn::Mul(l, v_mix);
                    const auto r_mix = hn::Mul(r, v_c_mix);
                    hn::StoreU(l_mix, d, right_l + i);
                    hn::StoreU(r_mix, d, right_r + i);
                    hn::StoreU(hn::Sub(l, l_mix), d, left_l + i);
                    hn::StoreU(hn::Sub(r, r_mix), d, left_r + i);
                }
                for (; i < num_samples; ++i) {
                    const auto l = in_l[i], r = in_r[i];
                    const auto l_mix = l * mix, r_mix = r * c_mix;
                    right_l[i] = l_mix;
                    right_r[i] = r_mix;
                    left_l[i] = l - l_mix;
                    left_r[i] = r - r_mix;
                }
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> mix_{};
    };
}
