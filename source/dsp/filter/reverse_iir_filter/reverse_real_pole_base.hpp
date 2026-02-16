// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <vector>

#include "../../delay/delay.hpp"

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
            std::fill(cs_.begin(), cs_.end(), FloatType(0));
            for (auto &delay: delays_) {
                for (auto &d: delay) {
                    d.reset();
                }
            }
        }

        void prepare(const size_t num_channels) {
            delays_.resize(num_channels);
            for (auto &delay: delays_) {
                int num_delay = 1;
                delay.resize(num_stage_ + 1);
                for (auto &d: delay) {
                    d.setMaximumDelayInSamples(num_delay);
                    d.setDelay(num_delay);
                    num_delay *= 2;
                }
            }
            reset();
        }

        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                auto &delay(delays_[chan]);
                auto chan_buffer = buffer[chan];
                for (int index = 0; index < static_cast<int>(num_samples); ++index) {
                    for (size_t stage = 0; stage <= num_stage_; ++stage) {
                        const auto current = chan_buffer[index];
                        chan_buffer[index] = cs_[stage] * current + delay[stage].push(current);
                    }
                }
            }
        }

        void updateFromBiquad(const std::array<double, 2> &a) {
            cs_[0] = -static_cast<FloatType>(a[1]);
            for (size_t i = 1; i <= num_stage_; ++i) {
                cs_[i] = cs_[i - 1] * cs_[i - 1];
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(num_stage_))) + 1;
        }

    private:
        size_t num_stage_{0};

        std::vector<std::vector<zldsp::delay::FIFODelay<FloatType>>> delays_;
        std::vector<FloatType> cs_;
    };
}