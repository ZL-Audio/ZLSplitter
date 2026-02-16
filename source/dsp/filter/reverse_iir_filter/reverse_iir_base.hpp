// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "reverse_cc_pole_base.hpp"

namespace zldsp::filter {
    /**
     * reverse of a second order IIR (with a pair of complex conjugates poles)
     * @tparam FloatType
     */
    template<typename FloatType>
    class ReverseIIRBase {
    public:
        explicit ReverseIIRBase(const size_t stage) : reverse_pole_(stage) {
        }

        void setNumStage(const size_t stage) {
            reverse_pole_.setNumStage(stage);
        }

        void reset() {
            reverse_pole_.reset();
            for (auto &state: states_) {
                std::fill(state.begin(), state.end(), FloatType(0));
            }
        }

        void prepare(const size_t max_num_samples, const size_t num_channels) {
            reverse_pole_.prepare(max_num_samples, num_channels);
            states_.resize(num_channels);
            reset();
        }

        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            reverse_pole_.process(buffer, num_samples);
            for (size_t chan = 0; chan < buffer.size(); ++chan) {
                std::array<FloatType, 2> &state(states_[chan]);
                auto chan_buffer = buffer[chan];
                for (size_t index = 0; index < num_samples; ++index) {
                    const auto current = chan_buffer[index];
                    chan_buffer[index] = zeros_[2] * current + zeros_[1] * state[0] + zeros_[0] * state[1];
                    state[1] = state[0];
                    state[0] = current;
                }
            }
        }

        void updateFromBiquad(const std::array<double, 6> &coeff) {
            const auto a0_inv = 1.0 / coeff[0];
            reverse_pole_.updateFromBiquad({1.0, coeff[1] * a0_inv, coeff[2] * a0_inv});
            zeros_[0] = static_cast<FloatType>(coeff[3] * a0_inv);
            zeros_[1] = static_cast<FloatType>(coeff[4] * a0_inv);
            zeros_[2] = static_cast<FloatType>(coeff[5] * a0_inv);
        }

        [[nodiscard]] inline int getLatency() const {
            return reverse_pole_.getLatency();
        }

    private:
        ReverseCCPoleBase<FloatType> reverse_pole_;
        std::vector<std::array<FloatType, 2>> states_;
        std::array<FloatType, 3> zeros_;
    };
}