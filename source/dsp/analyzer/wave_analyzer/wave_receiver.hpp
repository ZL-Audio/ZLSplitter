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
#include "../../vector/vector.hpp"
#include "../../container/fifo/fifo_base.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    class WaveReceiver {
    public:
        void run(const container::FIFORange range, const std::vector<std::vector<float>>& fifo) {
            min_gains_.resize(fifo.size());
            max_gains_.resize(fifo.size());
            for (size_t chan = 0; chan < fifo.size(); ++chan) {
                const auto bounds = calculateChannel<false, false>(range, fifo[chan], fifo[chan]);
                min_gains_[chan] = bounds[0];
                max_gains_[chan] = bounds[1];
            }
        }

        static std::array<float, 2> calculate(const container::FIFORange range,
                                              const std::vector<std::vector<float>>& fifo,
                                              const StereoType stereo_type) {
            if (range.block_size1 + range.block_size2 == 0) {
                return {0.f, 0.f};
            }
            if (fifo.size() != 2 || stereo_type == StereoType::kStereo) {
                std::array<float, 2> result{};
                for (const auto& channel : fifo) {
                    const auto bounds = calculateChannel<false, false>(range, channel, channel);
                    result[0] = std::min(result[0], bounds[0]);
                    result[1] = std::max(result[1], bounds[1]);
                }
                return result;
            }
            switch (stereo_type) {
            case StereoType::kLeft:
                return calculateChannel<false, false>(range, fifo[0], fifo[0]);
            case StereoType::kRight:
                return calculateChannel<false, false>(range, fifo[1], fifo[1]);
            case StereoType::kMid:
                return calculateChannel<true, false>(range, fifo[0], fifo[1]);
            case StereoType::kSide:
                return calculateChannel<true, true>(range, fifo[0], fifo[1]);
            case StereoType::kStereo:
                return {0.f, 0.f};
            }
            return {0.f, 0.f};
        }

        const std::vector<float>& getMinGains() const { return min_gains_; }

        const std::vector<float>& getMaxGains() const { return max_gains_; }

    private:
        std::vector<float> min_gains_;
        std::vector<float> max_gains_;

        // calculate both bounds with SIMD and a scalar tail
        template <bool Mix, bool IsSide>
        static std::array<float, 2> calculateChannel(const container::FIFORange range,
                                                    const std::vector<float>& left,
                                                    const std::vector<float>& right) {
            namespace hn = hwy::HWY_NAMESPACE;
            const hn::ScalableTag<float> d;
            const auto lanes = hn::Lanes(d);
            auto min_vec = hn::Zero(d);
            auto max_vec = hn::Zero(d);
            const auto scale = hn::Set(d, kSqrt2Over2);
            float min_value = 0.f, max_value = 0.f;
            const auto process = [&](const int start, const int count) {
                if (count <= 0) { return; }
                const auto size = static_cast<size_t>(count);
                const auto* l = left.data() + start;
                const auto* r = right.data() + start;
                size_t i = 0;
                for (; i + lanes <= size; i += lanes) {
                    auto value = hn::LoadU(d, l + i);
                    if constexpr (Mix) {
                        const auto right_value = hn::LoadU(d, r + i);
                        value = IsSide ? hn::Sub(value, right_value) : hn::Add(value, right_value);
                        value = hn::Mul(value, scale);
                    }
                    min_vec = hn::Min(min_vec, value);
                    max_vec = hn::Max(max_vec, value);
                }
                for (; i < size; ++i) {
                    auto value = l[i];
                    if constexpr (Mix) {
                        value = (IsSide ? value - r[i] : value + r[i]) * kSqrt2Over2;
                    }
                    min_value = std::min(min_value, value);
                    max_value = std::max(max_value, value);
                }
            };
            process(range.start_index1, range.block_size1);
            process(range.start_index2, range.block_size2);
            return {std::min(min_value, hn::ReduceMin(d, min_vec)),
                    std::max(max_value, hn::ReduceMax(d, max_vec))};
        }
    };
}
