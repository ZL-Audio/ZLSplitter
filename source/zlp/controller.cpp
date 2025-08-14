// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlp {
    template<typename FloatType>
    void Controller<FloatType>::prepare(const double sample_rate,
                                        const size_t max_num_samples) {
        lr_splitter_.prepare(sample_rate);
    }

    template<typename FloatType>
    void Controller<FloatType>::prepareBuffer() {
        if (!to_update_.exchange(false, std::memory_order::acquire)) {
            return;
        }
        if (to_update_split_type_.exchange(false, std::memory_order::acquire)) {
            c_split_type_ = split_type_.load(std::memory_order::relaxed);
        }
        if (to_update_mix_.exchange(false, std::memory_order::acquire)) {
            const auto mix = std::clamp(mix_.load(std::memory_order::relaxed),
                                        static_cast<FloatType>(0.0), static_cast<FloatType>(0.5));
            lr_splitter_.setMix(mix);
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::process(std::array<FloatType *, 2> &in_buffer,
                                        std::array<FloatType *, 4> &out_buffer,
                                        const size_t num_samples) {
        prepareBuffer();
        switch (c_split_type_) {
            case zlp::PSplitType::kLRight: {
                lr_splitter_.process(in_buffer, out_buffer, num_samples);
                break;
            }
            case zlp::PSplitType::kMSide:
            case zlp::PSplitType::kLHigh:
            case zlp::PSplitType::kTSteady:
            case zlp::PSplitType::kPSteady: {
                break;
            }
        }
    }

    template class Controller<float>;

    template class Controller<double>;
} // zlp
