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
                     std::array<FloatType *, 4> &out_buffer, const size_t num_samples) {
            const auto in_l = in_buffer[0], in_r = in_buffer[1];
            const auto out_l1 = out_buffer[0], out_r1 = out_buffer[1];
            const auto out_l2 = out_buffer[2], out_r2 = out_buffer[3];
            if (mix_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto mix = mix_.getNext();
                    const auto c_mix = static_cast<FloatType>(1.0) - mix;
                    const auto l = in_l[i], r = in_r[i];
                    out_l1[i] = l * c_mix;
                    out_r1[i] = r * mix;
                    out_l2[i] = l * mix;
                    out_r2[i] = r * c_mix;
                }
            } else {
                const auto mix = mix_.getCurrent();
                const auto c_mix = static_cast<FloatType>(1.0) - mix;
                zldsp::vector::multiply(out_l2, in_l, mix, num_samples);
                zldsp::vector::multiply(out_r2, in_r, c_mix, num_samples);
                zldsp::vector::multiply(out_l1, in_l, c_mix, num_samples);
                zldsp::vector::multiply(out_r1, in_r, mix, num_samples);
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> mix_{};
    };
}
