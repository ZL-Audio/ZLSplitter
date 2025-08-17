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
    Controller<FloatType>::Controller(juce::AudioProcessor &processor)
        : p_ref_(processor) {
    }

    template<typename FloatType>
    void Controller<FloatType>::prepare(const double sample_rate,
                                        const size_t max_num_samples) {
        lr_splitter_.prepare(sample_rate);
        ms_splitter_.prepare(sample_rate);
        lh_splitter_.prepare(sample_rate, 2);
        lh_fir_splitter_.prepare(sample_rate, 2, max_num_samples);
        ts_splitter_[0].prepare(sample_rate, 1, max_num_samples);
        ts_splitter_[1].prepare(sample_rate, 1, max_num_samples);
        ps_splitter_[0].prepare(sample_rate);
        ps_splitter_[1].prepare(sample_rate);
    }

    template<typename FloatType>
    void Controller<FloatType>::prepareBuffer() {
        if (to_update_.exchange(false, std::memory_order::acquire)) {
            if (to_update_split_type_.exchange(false, std::memory_order::acquire)) {
                c_split_type_ = split_type_.load(std::memory_order::relaxed);
            }
            c_use_fir_ = use_fir_.load(std::memory_order::relaxed);
            switch (c_split_type_) {
                case zlp::PSplitType::kLRight:
                case zlp::PSplitType::kMSide: {
                    latency_.store(0, std::memory_order::relaxed);
                    break;
                }
                case zlp::PSplitType::kLHigh: {
                    if (c_use_fir_) {
                        lh_fir_splitter_.prepareBuffer();
                        latency_.store(lh_fir_splitter_.getLatency(), std::memory_order::relaxed);
                    } else {
                        latency_.store(0, std::memory_order::relaxed);
                    }
                    break;
                }
                case zlp::PSplitType::kTSteady: {
                    latency_.store(ts_splitter_[0].getTSLatency(), std::memory_order::relaxed);
                    break;
                }
                case zlp::PSplitType::kPSteady: {
                    latency_.store(0, std::memory_order::relaxed);
                    break;
                }
            }
            checkUpdateLatency();
        }
        if (to_update_mix_.exchange(false, std::memory_order::acquire)) {
            const auto mix = std::clamp(mix_.load(std::memory_order::relaxed),
                                        static_cast<FloatType>(0.0), static_cast<FloatType>(0.5));
            lr_splitter_.setMix(mix);
            ms_splitter_.setMix(mix);
            lh_splitter_.setMix(mix);
            lh_fir_splitter_.setMix(mix);
        }
        switch (c_split_type_) {
            case zlp::PSplitType::kLRight:
            case zlp::PSplitType::kMSide: {
                break;
            }
            case zlp::PSplitType::kLHigh: {
                if (c_use_fir_) {
                    lh_fir_splitter_.prepareBuffer();
                } else {
                    lh_splitter_.prepareBuffer();
                }
                break;
            }
            case zlp::PSplitType::kTSteady: {
                break;
            }
            case zlp::PSplitType::kPSteady: {
                ps_splitter_[0].prepareBuffer();
                ps_splitter_[1].prepareBuffer();
                break;
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::process(std::array<FloatType *, 2> &in_buffer,
                                        std::array<FloatType *, 4> &out_buffer,
                                        const size_t num_samples) {
        prepareBuffer();
        out_buffer1_[0] = out_buffer[0];
        out_buffer1_[1] = out_buffer[1];
        out_buffer2_[0] = out_buffer[2];
        out_buffer2_[1] = out_buffer[3];

        switch (c_split_type_) {
            case zlp::PSplitType::kLRight: {
                lr_splitter_.process(in_buffer, out_buffer1_, out_buffer2_, num_samples);
                break;
            }
            case zlp::PSplitType::kMSide: {
                ms_splitter_.process(in_buffer, out_buffer1_, out_buffer2_, num_samples);
                break;
            }
            case zlp::PSplitType::kLHigh: {
                if (use_fir_) {
                    lh_fir_splitter_.process(in_buffer, out_buffer1_, out_buffer2_, num_samples);
                } else {
                    lh_splitter_.process(in_buffer, out_buffer1_, out_buffer2_, num_samples);
                }
                break;
            }
            case zlp::PSplitType::kTSteady: {
                ts_splitter_[0].process(in_buffer[0], out_buffer1_[0], out_buffer2_[0], num_samples);
                ts_splitter_[1].process(in_buffer[1], out_buffer1_[1], out_buffer2_[1], num_samples);
                break;
            }
            case zlp::PSplitType::kPSteady: {
                ps_splitter_[0].process(in_buffer[0], out_buffer1_[0], out_buffer2_[0], num_samples);
                ps_splitter_[1].process(in_buffer[1], out_buffer1_[1], out_buffer2_[1], num_samples);
                break;
            }
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::checkUpdateLatency() {
        if (latency_.load(std::memory_order::relaxed) != p_ref_.getLatencySamples()) {
            triggerAsyncUpdate();
        }
    }

    template<typename FloatType>
    void Controller<FloatType>::handleAsyncUpdate() {
        p_ref_.setLatencySamples(latency_.load(std::memory_order::relaxed));
    }

    template class Controller<float>;

    template class Controller<double>;
} // zlp
