// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../chore/decibels.hpp"
#include "style_base.hpp"

namespace zldsp::compressor {
    template<typename FloatType>
    class ClassicCompressor final : public CompressorStyleBase<FloatType> {
    public:
        using base = CompressorStyleBase<FloatType>;

        ClassicCompressor(ComputerBase<FloatType> &computer,
                          RMSTracker<FloatType> &tracker,
                          FollowerBase<FloatType> &follower)
            : base(computer, tracker, follower) {
        }

        void reset() {
            base::follower_.reset(FloatType(0));
            x0_ = FloatType(0);
        }

        template <bool UseRMS = false>
        void process(FloatType *buffer, const size_t num_samples) {
            for (size_t i = 0; i < num_samples; ++i) {
                FloatType input_db;
                if constexpr (UseRMS) {
                    // pass the feedback sample through the tracker
                    base::tracker_.processSample(x0_);
                    // get the db from the tracker
                    input_db = base::tracker_.getMomentaryDB();
                } else {
                    input_db = chore::gainToDecibels(std::abs(x0_));
                }
                // pass through the computer and the follower
                const auto smooth_reduction_db = -base::follower_.processSample(-base::computer_.eval(input_db));
                // apply the gain on the current sample and save it as the feedback sample for the next
                x0_ = buffer[i] * chore::decibelsToGain(smooth_reduction_db);
                buffer[i] = smooth_reduction_db;
            }
        }

    private:
        FloatType x0_{FloatType(0)};
    };
}
