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
#include "../dsp/fft_analyzer/fft_analyzer.hpp"
#include "../dsp/mag_analyzer/mag_analyzer.hpp"
#include "zlp_definitions.hpp"

namespace zlp {
    template<typename FloatType>
    class Controller : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;

        explicit Controller(juce::AudioProcessor &processor);

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
        }

        void setLHOrder(size_t order) {
            lh_splitter_.setOrder(order);
            lh_fir_splitter_.setOrder(order);
            to_update_.store(true, std::memory_order::release);
        }

        void setUseFIR(const bool f) {
            use_fir_.store(f, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        zldsp::splitter::LHSplitter<FloatType> &getLHSplitter() {
            return lh_splitter_;
        }

        zldsp::splitter::LHFIRSplitter<FloatType> &getLHFIRSplitter() {
            return lh_fir_splitter_;
        }

        std::array<zldsp::splitter::TSSplitter<FloatType>, 2> &getTSSplitter() {
            return ts_splitter_;
        }

        std::array<zldsp::splitter::PSSplitter<FloatType>, 2> &getPSSplitter() {
            return ps_splitter_;
        }

        void setFFTAnalyzerOn(const bool f) {
            is_fft_on_.store(f, std::memory_order::relaxed);
        }

        zldsp::analyzer::MultipleFFTAnalyzer<FloatType, 2, kAnalyzerPointNum> &getFFTAnalyzer() {
            return fft_analyzer_;
        }

        void setMagAnalyzerOn(const bool f) {
            is_mag_on_.store(f, std::memory_order::relaxed);
        }

        zldsp::analyzer::MultipleMagAnalyzer<FloatType, 2, kAnalyzerPointNum> &getMagAnalyzer() {
            return mag_analyzer_;
        }

    private:
        juce::AudioProcessor &p_ref_;
        std::array<FloatType *, 2> out_buffer1_, out_buffer2_;
        zldsp::splitter::LRSplitter<FloatType> lr_splitter_;
        zldsp::splitter::MSSplitter<FloatType> ms_splitter_;
        zldsp::splitter::LHSplitter<FloatType> lh_splitter_;
        zldsp::splitter::LHFIRSplitter<FloatType> lh_fir_splitter_;
        std::array<zldsp::splitter::TSSplitter<FloatType>, 2> ts_splitter_;
        std::array<zldsp::splitter::PSSplitter<FloatType>, 2> ps_splitter_;

        std::atomic<bool> to_update_{true};

        std::atomic<zlp::PSplitType::SplitType> split_type_{PSplitType::SplitType::kLRight};
        zlp::PSplitType::SplitType c_split_type_{PSplitType::SplitType::kLRight};
        std::atomic<bool> to_update_split_type_{false};

        std::atomic<FloatType> mix_{};
        std::atomic<bool> to_update_mix_{false};

        std::atomic<bool> use_fir_{false};
        bool c_use_fir_{false};

        std::atomic<int> latency_{0};

        std::atomic<bool> is_fft_on_{false};
        std::array<std::span<FloatType*>, 2> analyzer_spans_;
        zldsp::analyzer::MultipleFFTAnalyzer<FloatType, 2, kAnalyzerPointNum> fft_analyzer_;

        std::atomic<bool> is_mag_on_{false};
        zldsp::analyzer::MultipleMagAnalyzer<FloatType, 2, kAnalyzerPointNum> mag_analyzer_;

        void checkUpdateLatency();

        void handleAsyncUpdate() override;
    };
}
