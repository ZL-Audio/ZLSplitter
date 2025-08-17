// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <vector>

namespace zldsp::delay {
    template <typename FloatType>
    class FIFODelay {
    public:
        FIFODelay() {
            setMaximumDelayInSamples(1);
        }

        void setMaximumDelayInSamples(const int x) {
            maximum_delay_ = x;
            state_.resize(static_cast<size_t>(x));
            reset();
        }

        void setDelay(const int x) {
            num_delay_ = x;
        }

        int getDelay() const {
            return num_delay_;
        }

        void reset() {
            std::fill(state_.begin(), state_.end(), FloatType(0));
            pos_ = 0;
        }

        FloatType push(FloatType x) {
            const auto current = state_[static_cast<size_t>(pos_)];
            state_[static_cast<size_t>(pos_)] = x;
            pos_ = (pos_ + 1) % num_delay_;
            return current;
        }

    private:
        int maximum_delay_{1}, num_delay_{1};
        std::vector<FloatType> state_;
        int pos_{0};
    };
}
