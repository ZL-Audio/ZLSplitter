// Copyright (C) 2025 - zsliu98
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
#include "../../vector/vector.hpp"

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

        void process(std::array<FloatType *, 2> &in_buffer,
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
                const auto mix = mix_.getCurrent();
                const auto c_mix = static_cast<FloatType>(1.0) - mix;
                auto in_l_v = kfr::make_univector(in_l, num_samples);
                auto in_r_v = kfr::make_univector(in_r, num_samples);
                auto left_l_v = kfr::make_univector(left_l, num_samples);
                auto left_r_v = kfr::make_univector(left_r, num_samples);
                auto right_l_v = kfr::make_univector(right_l, num_samples);
                auto right_r_v = kfr::make_univector(right_r, num_samples);

                right_l_v = in_l_v * mix;
                right_r_v = in_r_v * c_mix;
                left_l_v = in_l_v - right_l_v;
                left_r_v = in_r_v - right_r_v;
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> mix_{};
    };
}
