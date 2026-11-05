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
#include <cmath>
#include <span>
#include <vector>

#include "../../delay/block_fifo_delay.hpp"
#include "../../vector/highway_import.hpp"

namespace zldsp::filter {
    /**
     * reverse of a pair of complex conjugates poles
     * @tparam FloatType
     */
    template<typename FloatType>
    class ReverseCCPoleBase {
    public:
        explicit ReverseCCPoleBase(const size_t stage) {
            setNumStage(stage);
        }

        void setNumStage(const size_t stage) {
            num_stage_ = stage;
            a_state_.resize(stage + 1);
            b_state_.resize(stage + 1);
        }

        void reset() {
            for (auto& delay : input_delays_) {
                delay.reset();
            }
            for (auto& channel : delays_) {
                for (auto& delay : channel) {
                    delay.reset();
                }
            }
        }

        void prepare(const size_t max_num_samples, const size_t num_channels) {
            max_num_samples_ = std::max(size_t(1), max_num_samples);
            v_buffer_.resize(max_num_samples_);
            input_delays_.resize(num_channels);
            for (auto& delay : input_delays_) {
                delay.prepare(1, max_num_samples_);
            }
            delays_.resize(num_channels);
            for (auto& channel : delays_) {
                channel.resize(num_stage_);
                for (size_t stage = 1; stage <= num_stage_; ++stage) {
                    channel[stage - 1].prepare(size_t(1) << stage, max_num_samples_);
                }
            }
        }

        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            if (is_complex_) {
                processComplex(buffer, num_samples);
            }
        }

        void updateFromBiquad(const std::array<double, 3> &a) {
            const auto delta = a[1] * a[1] / 4 - a[2];
            is_complex_ = (delta < 0);
            if (is_complex_) {
                a_state_[0] = -static_cast<FloatType>(a[1] / 2);
                b_state_[0] = -static_cast<FloatType>(std::sqrt(a[2] - a[1] * a[1] / 4));
                aB_ = a_state_[0] / b_state_[0];
                for (size_t i = 1; i <= num_stage_; ++i) {
                    const auto ii = i - 1;
                    a_state_[i] = a_state_[ii] * a_state_[ii] - b_state_[ii] * b_state_[ii];
                    b_state_[i] = 2 * a_state_[ii] * b_state_[ii];
                }
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(num_stage_))) - 1;
        }

    private:
        size_t num_stage_{0};
        size_t max_num_samples_{1};
        std::vector<zldsp::delay::BlockFIFODelay<FloatType>> input_delays_;
        std::vector<std::vector<zldsp::delay::BlockFIFODelay<FloatType, 2>>> delays_;
        std::vector<FloatType> v_buffer_;
        std::vector<FloatType> a_state_, b_state_;
        FloatType aB_{0};

        bool is_complex_{false};

        void processComplex(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t offset = 0; offset < num_samples;) {
                const auto count = std::min(max_num_samples_, num_samples - offset);
                for (size_t chan = 0; chan < buffer.size(); ++chan) {
                    auto* u = buffer[chan] + offset;
                    auto* v = v_buffer_.data();
                    const auto input = input_delays_[chan].push({u}, count);
                    processInput(u, v, input[0].data[0], a_state_[0], b_state_[0], input[0].size);
                    processInput(u + input[0].size, v + input[0].size, input[1].data[0],
                                 a_state_[0], b_state_[0], input[1].size);
                    for (size_t stage = 1; stage <= num_stage_; ++stage) {
                        const auto block = delays_[chan][stage - 1].push({u, v}, count);
                        processStage(u, v, block[0].data[0], block[0].data[1],
                                     a_state_[stage], b_state_[stage], block[0].size);
                        processStage(u + block[0].size, v + block[0].size, block[1].data[0], block[1].data[1],
                                     a_state_[stage], b_state_[stage], block[1].size);
                    }
                    for (size_t i = 0; i < count; ++i) {
                        u[i] += aB_ * v[i];
                    }
                }
                offset += count;
            }
        }

        static void processInput(FloatType* HWY_RESTRICT u, FloatType* HWY_RESTRICT v,
                                 const FloatType* delayed,
                                 const FloatType a, const FloatType b, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto current = u[i];
                u[i] = a * current + delayed[i];
                v[i] = b * current;
            }
        }

        static void processStage(FloatType* HWY_RESTRICT u, FloatType* HWY_RESTRICT v,
                                 const FloatType* u_delayed, const FloatType* v_delayed,
                                 const FloatType a, const FloatType b, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                const auto u_current = u[i];
                const auto v_current = v[i];
                u[i] = a * u_current - b * v_current + u_delayed[i];
                v[i] = b * u_current + a * v_current + v_delayed[i];
            }
        }
    };
}
