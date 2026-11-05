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
#include <span>
#include <vector>

#include "../../delay/block_fifo_delay.hpp"
#include "../../vector/highway_import.hpp"

namespace zldsp::filter {
    /**
     * reverse of a single real pole
     * @tparam FloatType
     */
    template<typename FloatType>
    class ReverseRealPoleBase {
    public:
        explicit ReverseRealPoleBase(const size_t num_stage) {
            setNumStage(num_stage);
        }

        void setNumStage(const size_t num_stage) {
            num_stage_ = num_stage;
            cs_.resize(num_stage + 1);
        }

        void reset() {
            for (auto &delay: delays_) {
                for (auto &d: delay) {
                    d.reset();
                }
            }
        }

        void prepare(const size_t max_num_samples, const size_t num_channels) {
            max_num_samples_ = std::max(size_t(1), max_num_samples);
            delays_.resize(num_channels);
            for (auto &delay: delays_) {
                delay.resize(num_stage_ + 1);
                for (size_t stage = 0; stage <= num_stage_; ++stage) {
                    delay[stage].prepare(size_t(1) << stage, max_num_samples_);
                }
            }
        }

        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t offset = 0; offset < num_samples;) {
                const auto count = std::min(max_num_samples_, num_samples - offset);
                for (size_t chan = 0; chan < buffer.size(); ++chan) {
                    auto* output = buffer[chan] + offset;
                    for (size_t stage = 0; stage <= num_stage_; ++stage) {
                        const auto block = delays_[chan][stage].push({output}, count);
                        processStage(output, block[0].data[0], cs_[stage], block[0].size);
                        processStage(output + block[0].size, block[1].data[0], cs_[stage], block[1].size);
                    }
                }
                offset += count;
            }
        }

        void updateFromBiquad(const std::array<double, 2> &a) {
            cs_[0] = -static_cast<FloatType>(a[1]);
            for (size_t i = 1; i <= num_stage_; ++i) {
                cs_[i] = cs_[i - 1] * cs_[i - 1];
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(num_stage_))) - 1;
        }

    private:
        size_t num_stage_{0};
        size_t max_num_samples_{1};

        std::vector<std::vector<zldsp::delay::BlockFIFODelay<FloatType>>> delays_;
        std::vector<FloatType> cs_;

        static void processStage(FloatType* HWY_RESTRICT output,
                                 const FloatType* delayed,
                                 const FloatType coefficient, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                output[i] = coefficient * output[i] + delayed[i];
            }
        }
    };
}
