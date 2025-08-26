// Copyright (C) 2025 - zsliu98
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
            for (auto &buffer: u_state_) {
                std::fill(buffer.begin(), buffer.end(), FloatType(0));
            }

            for (auto &buffer: v_state_) {
                std::fill(buffer.begin(), buffer.end(), FloatType(0));
            }

            for (auto &delay: u_delays_) {
                for (auto &d: delay) {
                    d.reset();
                }
            }

            for (auto &delay: v_delays_) {
                for (auto &d: delay) {
                    d.reset();
                }
            }
        }

        void prepare(const size_t max_num_samples, const size_t num_channels) {
            u_state_.resize(num_channels);
            v_state_.resize(num_channels);
            u_delays_.resize(num_channels);
            v_delays_.resize(num_channels);

            for (auto &buffer: u_state_) {
                buffer.resize(max_num_samples);
            }

            for (auto &buffer: v_state_) {
                buffer.resize(max_num_samples);
            }

            for (auto &delay: u_delays_) {
                int num_delay = 1;
                delay.resize(num_stage_ + 1);
                for (size_t i = 0; i < num_stage_ + 1; ++i) {
                    delay[i].setMaximumDelayInSamples(num_delay);
                    delay[i].setDelay(num_delay);
                    num_delay *= 2;
                }
            }

            for (auto &delay: v_delays_) {
                int num_delay = 1;
                delay.resize(num_stage_ + 1);
                for (size_t i = 0; i < num_stage_ + 1; ++i) {
                    delay[i].setMaximumDelayInSamples(num_delay);
                    delay[i].setDelay(num_delay);
                    num_delay *= 2;
                }
            }
            reset();
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
                for (size_t i = 1; i < num_stage_; ++i) {
                    const auto ii = i - 1;
                    a_state_[i] = a_state_[ii] * a_state_[ii] - b_state_[ii] * b_state_[ii];
                    b_state_[i] = 2 * a_state_[ii] * b_state_[ii];
                }
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(num_stage_))) + 1;
        }

    private:
        size_t num_stage_{0};
        std::vector<std::vector<zldsp::delay::FIFODelay<FloatType> > > u_delays_, v_delays_;
        std::vector<FloatType> a_state_, b_state_;
        FloatType aB_;
        std::vector<std::vector<FloatType> > u_state_, v_state_;

        bool is_complex_{false};

        void processComplex(std::span<FloatType *> buffer, const size_t num_samples) {
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                std::vector<FloatType> &u(u_state_[chan]), &v(v_state_[chan]);

                auto &u_delay(u_delays_[chan]);
                auto &v_delay(v_delays_[chan]);

                auto chan_buffer = buffer[chan];
                for (size_t index = 0; index < num_samples; ++index) {
                    {
                        const auto current = chan_buffer[index];
                        u[index] = a_state_[0] * current + u_delay[0].push(current);
                        v[index] = b_state_[0] * current;
                    }
                    for (size_t stage = 1; stage <= num_stage_; ++stage) {
                        const auto u_current = u[index];
                        const auto v_current = v[index];
                        u[index] = a_state_[stage] * u_current - b_state_[stage] * v_current + u_delay[stage].push(
                                       u_current);
                        v[index] = b_state_[stage] * u_current + a_state_[stage] * v_current + v_delay[stage].push(
                                       v_current);
                    }
                    chan_buffer[index] = u[index] + aB_ * v[index];
                }
            }
        }
    };
}
