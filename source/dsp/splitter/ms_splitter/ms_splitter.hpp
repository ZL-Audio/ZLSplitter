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
    class MSSplitter {
    public:
        MSSplitter() = default;

        void prepare(const double sample_rate) {
            c_mix_.prepare(sample_rate, 0.1);
        }

        void setMix(const FloatType mix) {
            c_mix_.setTarget(static_cast<FloatType>(0.5) - mix);
        }

        void process(std::array<FloatType *, 2> &in_buffer,
                     std::array<FloatType *, 4> &out_buffer, const size_t num_samples) {
            const auto in_l = in_buffer[0], in_r = in_buffer[1];
            const auto out_l1 = out_buffer[0], out_r1 = out_buffer[1];
            const auto out_l2 = out_buffer[2], out_r2 = out_buffer[3];
            if (c_mix_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto c_mix = c_mix_.getNext();
                    const auto l = in_l[i], r = in_r[i];
                    const auto l_mix = static_cast<FloatType>(0.5) * l - c_mix * r;
                    const auto r_mix = static_cast<FloatType>(0.5) * r - c_mix * l;
                    out_l2[i] = l_mix;
                    out_r2[i] = r_mix;
                    out_l1[i] = l - l_mix;
                    out_r1[i] = r - r_mix;
                }
            } else {
                const auto c_mix = c_mix_.getCurrent();

                auto in_l_v = kfr::make_univector(in_l, num_samples);
                auto in_r_v = kfr::make_univector(in_r, num_samples);
                auto out_l1_v = kfr::make_univector(out_l1, num_samples);
                auto out_r1_v = kfr::make_univector(out_r1, num_samples);
                auto out_l2_v = kfr::make_univector(out_l2, num_samples);
                auto out_r2_v = kfr::make_univector(out_r2, num_samples);

                out_l2_v = static_cast<FloatType>(0.5) * in_l_v - c_mix * in_r_v;
                out_r2_v = static_cast<FloatType>(0.5) * in_r_v - c_mix * in_l_v;
                out_l1_v = in_l_v - out_l2_v;
                out_r1_v = in_r_v - out_r2_v;
            }
        }

    private:
        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> c_mix_{static_cast<FloatType>(0.5)};
    };
}
