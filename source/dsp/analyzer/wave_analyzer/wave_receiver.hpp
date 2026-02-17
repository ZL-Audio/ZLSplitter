// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../mag_analyzer/mag_receiver_base.hpp"
#include "../analyzer_base/analyzer_receiver_base.hpp"

namespace zldsp::analyzer {
    class WaveReceiver {
    public:
        void run(const zldsp::container::FIFORange range,
                 const std::vector<std::vector<float>>& fifo) {
            min_gains_.resize(fifo.size());
            max_gains_.resize(fifo.size());
            for (size_t chan = 0; chan < fifo.size(); ++chan) {
                min_gains_[chan] = MagAnalyzerOps::calculateMin(range, fifo[chan]);
                max_gains_[chan] = MagAnalyzerOps::calculateMax(range, fifo[chan]);
            }
        }

        static std::array<float, 2> calculate(const zldsp::container::FIFORange range,
                                              const std::vector<std::vector<float>>& fifo,
                                              const StereoType stereo_type) {
            if (range.block_size1 + range.block_size2 == 0) {
                return {0.f, 0.f};
            }
            if (fifo.size() != 2 || stereo_type == StereoType::kStereo) {
                float min_v{0.f}, max_v{0.f};
                for (size_t chan = 0; chan < fifo.size(); ++chan) {
                    max_v = std::max(max_v, MagAnalyzerOps::calculateMax(range, fifo[chan]));
                    min_v = std::min(min_v, MagAnalyzerOps::calculateMin(range, fifo[chan]));
                }
                return {min_v, max_v};
            }
            float min_v{0.f}, max_v{0.f};
            auto analyze_expr = [&](const auto& expression) {
                min_v = std::min(min_v, kfr::minof(expression));
                max_v = std::max(max_v, kfr::maxof(expression));
            };
            auto process_segment = [&](const size_t start, const size_t size) {
                if (size == 0) {
                    return;
                }
                auto vL = kfr::make_univector(fifo[0].data() + start, size);
                auto vR = kfr::make_univector(fifo[1].data() + start, size);

                switch (stereo_type) {
                case StereoType::kLeft:
                    analyze_expr(vL);
                    break;
                case StereoType::kRight:
                    analyze_expr(vR);
                    break;
                case StereoType::kMid:
                    analyze_expr(kSqrt2Over2 * (vL + vR));
                    break;
                case StereoType::kSide:
                    analyze_expr(kSqrt2Over2 * (vL - vR));
                    break;
                case StereoType::kStereo:
                default:
                    break;
                }
            };
            process_segment(static_cast<size_t>(range.start_index1), static_cast<size_t>(range.block_size1));
            process_segment(static_cast<size_t>(range.start_index2), static_cast<size_t>(range.block_size2));
            return {min_v, max_v};
        }

        const std::vector<float>& getMinGains() { return min_gains_; }

        const std::vector<float>& getMaxGains() { return max_gains_; }

    private:
        std::vector<float> min_gains_;
        std::vector<float> max_gains_;
    };
}
