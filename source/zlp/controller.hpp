// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>
#include <algorithm>

#include <juce_audio_processors/juce_audio_processors.h>

#include "../dsp/splitter/splitter.hpp"
#include "zlp_definitions.hpp"

namespace zlp {
    template<typename FloatType>
    class Controller {
    public:
        explicit Controller() = default;

        void prepare(double sample_rate, size_t max_num_samples);

        void prepareBuffer();

        void process(std::array<FloatType *, 2> &in_buffer,
                     std::array<FloatType *, 4> &out_buffer,
                     size_t num_samples);

        void setSplitType(zlp::PSplitType::SplitType split_type) {
            split_type_.store(split_type, std::memory_order::relaxed);
            to_update_split_type_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

        void setMix(FloatType mix) {
            mix_.store(mix, std::memory_order::relaxed);
            to_update_mix_.store(true, std::memory_order::release);
            to_update_.store(true, std::memory_order::release);
        }

    private:
        zldsp::splitter::LRSplitter<FloatType> lr_splitter_;
        zldsp::splitter::MSSplitter<FloatType> ms_splitter_;

        std::atomic<bool> to_update_{false};

        std::atomic<zlp::PSplitType::SplitType> split_type_{PSplitType::SplitType::kLRight};
        zlp::PSplitType::SplitType c_split_type_{PSplitType::SplitType::kLRight};
        std::atomic<bool> to_update_split_type_{false};

        std::atomic<FloatType> mix_{};
        std::atomic<bool> to_update_mix_{false};
    };
}
