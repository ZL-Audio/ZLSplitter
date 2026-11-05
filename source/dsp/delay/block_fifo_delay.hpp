// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <vector>

namespace zldsp::delay {
    /**
     * Circular input history for one or more streams sharing a delay.
     * Each push returns the delayed input in at most two contiguous spans.
     * The returned pointers remain valid until the next push, reset or prepare.
     */
    template<typename FloatType, size_t NumStreams = 1>
    class BlockFIFODelay {
    public:
        struct Span {
            std::array<const FloatType*, NumStreams> data;
            size_t size;
        };

        using Block = std::array<Span, 2>;

        void prepare(const size_t delay, const size_t max_num_samples) {
            delay_ = delay;
            max_num_samples_ = std::max(size_t(1), max_num_samples);
            // Reserve space for the entire incoming block so that appending it
            // cannot overwrite the history still needed by the delayed read.
            capacity_ = delay_ + max_num_samples_;
            for (auto& state : states_) {
                state.resize(capacity_);
            }
            reset();
        }

        void reset() {
            for (auto& state : states_) {
                std::fill(state.begin(), state.end(), FloatType(0));
            }
            write_position_ = 0;
        }

        Block push(const std::array<const FloatType*, NumStreams>& input,
                   const size_t num_samples) {
            assert(num_samples <= max_num_samples_);
            const auto first_write_size = std::min(num_samples, capacity_ - write_position_);
            const auto second_write_size = num_samples - first_write_size;
            const auto read_position = write_position_ >= delay_ ? write_position_ - delay_
                : capacity_ - (delay_ - write_position_);
            const auto first_read_size = std::min(num_samples, capacity_ - read_position);

            Block block;
            block[0].size = first_read_size;
            block[1].size = num_samples - first_read_size;
            for (size_t stream = 0; stream < NumStreams; ++stream) {
                auto* state = states_[stream].data();
                if (first_write_size != 0) {
                    std::memcpy(state + write_position_, input[stream], first_write_size * sizeof(FloatType));
                }
                if (second_write_size != 0) {
                    std::memcpy(state, input[stream] + first_write_size, second_write_size * sizeof(FloatType));
                }
                block[0].data[stream] = state + read_position;
                block[1].data[stream] = state;
            }
            write_position_ += num_samples;
            if (write_position_ >= capacity_) {
                write_position_ -= capacity_;
            }
            return block;
        }

    private:
        std::array<std::vector<FloatType>, NumStreams> states_;
        size_t delay_{0}, max_num_samples_{1}, capacity_{0}, write_position_{0};
    };
}
