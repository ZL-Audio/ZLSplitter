// Copyright (C) 2026 - zsliu98
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

#include "../chore/thread/notifier.hpp"
#include "../dsp/splitter/splitter.hpp"
#include "../dsp/analyzer/analyzer_base/analyzer_sender_base.hpp"
#include "zlp_definitions.hpp"

namespace zlp {
    template <typename FloatType>
    class Controller final : private juce::AsyncUpdater {
    public:
        static constexpr size_t kAnalyzerPointNum = 251;

        explicit Controller(juce::AudioProcessor& processor);

        void prepare(double sample_rate, size_t max_num_samples);

        void prepareBuffer();

        void process(std::array<FloatType*, 2>& in_buffer,
                     std::array<FloatType*, 4>& out_buffer,
                     size_t num_samples);

        void processBypassDelay(std::array<FloatType*, 2>& in_buffer, size_t num_samples);

        void setSplitType(const zlp::PSplitType::SplitType split_type) {
            split_type_.store(split_type, std::memory_order::relaxed);
            to_update_config_.signal();
            to_update_.signal();
        }

        void setMix(const FloatType mix) {
            mix_.store(mix, std::memory_order::relaxed);
            to_update_mix_.signal();
            to_update_.signal();
        }

        void setLHOrder(const size_t order) {
            lh_order_.store(order, std::memory_order::relaxed);
            to_update_lh_.signal();
            to_update_.signal();
        }

        void setLHFreq(const double freq) {
            lh_freq_.store(freq, std::memory_order::relaxed);
            to_update_lh_.signal();
            to_update_.signal();
        }

        void setUseFIR(const bool f) {
            use_fir_.store(f, std::memory_order::relaxed);
            to_update_config_.signal();
            to_update_.signal();
        }

        void setTSBalance(const float x) {
            ts_balance_.store(x, std::memory_order::relaxed);
            to_update_ts_.signal();
            to_update_.signal();
        }

        void setTSSeparation(const float x) {
            ts_separation_.store(x, std::memory_order::relaxed);
            to_update_ts_.signal();
            to_update_.signal();
        }

        void setTSHold(const float x) {
            ts_hold_.store(x, std::memory_order::relaxed);
            to_update_ts_.signal();
            to_update_.signal();
        }

        void setTSSmooth(const float x) {
            ts_smooth_.store(x, std::memory_order::relaxed);
            to_update_ts_.signal();
            to_update_.signal();
        }

        void setPSBalance(const FloatType x) {
            ps_balance_.store(x, std::memory_order::relaxed);
            to_update_ps_.signal();
            to_update_.signal();
        }

        void setPSAttack(const FloatType x) {
            ps_attack_.store(x, std::memory_order::relaxed);
            to_update_ps_.signal();
            to_update_.signal();
        }

        void setPSHold(const FloatType x) {
            ps_hold_.store(x, std::memory_order::relaxed);
            to_update_ps_.signal();
            to_update_.signal();
        }

        void setPSSmooth(const FloatType x) {
            ps_smooth_.store(x, std::memory_order::relaxed);
            to_update_ps_.signal();
            to_update_.signal();
        }

        void setAnalyzerOn(const bool f) {
            analyzer_on_.store(f, std::memory_order::relaxed);
            to_update_.signal();
        }

        auto& getAnalyzerSender() {
            return analyzer_sender_;
        }

    private:
        juce::AudioProcessor& p_ref_;
        std::array<FloatType*, 2> out_buffer1_, out_buffer2_;
        zldsp::splitter::LRSplitter<FloatType> lr_splitter_;
        zldsp::splitter::MSSplitter<FloatType> ms_splitter_;
        zldsp::splitter::LHSplitter<FloatType> lh_splitter_;
        zldsp::splitter::LHFIRSplitter<FloatType> lh_fir_splitter_;
        std::array<zldsp::splitter::TSSplitter<FloatType>, 2> ts_splitter_;
        std::array<zldsp::splitter::PSSplitter<FloatType>, 2> ps_splitter_;

        zlchore::thread::Notifier to_update_{true};

        std::atomic<zlp::PSplitType::SplitType> split_type_{PSplitType::SplitType::kLRight};
        std::atomic<bool> use_fir_{false};
        zlchore::thread::Notifier to_update_config_{true};
        zlp::PSplitType::SplitType c_split_type_{PSplitType::SplitType::kLRight};
        bool c_use_fir_{false};

        std::atomic<FloatType> mix_{};
        zlchore::thread::Notifier to_update_mix_{true};

        std::atomic<double> lh_freq_{PLHFreq::kDefaultV};
        std::atomic<size_t> lh_order_{PLHSlope::kOrders[PLHSlope::kDefaultI]};
        zlchore::thread::Notifier to_update_lh_{true};

        std::atomic<float> ts_balance_{PTSBalance::kDefaultV / 100.f + .5f};
        std::atomic<float> ts_separation_{PTSStrength::kDefaultV / 100.f};
        std::atomic<float> ts_hold_{PTSHold::kDefaultV / 100.f};
        std::atomic<float> ts_smooth_{PTSSmooth::kDefaultV / 100.f};
        zlchore::thread::Notifier to_update_ts_{true};

        std::atomic<FloatType> ps_balance_{PPSBalance::kDefaultV / 100.f + .5f};
        std::atomic<FloatType> ps_attack_{PPSAttack::kDefaultV / 100.f};
        std::atomic<FloatType> ps_hold_{PPSHold::kDefaultV / 100.f};
        std::atomic<FloatType> ps_smooth_{PPSSmooth::kDefaultV / 100.f};
        zlchore::thread::Notifier to_update_ps_{true};

        std::atomic<int> latency_{0};

        std::atomic<bool> analyzer_on_{false};
        bool c_analyzer_on_{false};
        zldsp::analyzer::AnalyzerSenderBase<FloatType, 2> analyzer_sender_{};

        zldsp::delay::IntegerDelay<FloatType> bypass_delay_;

        bool updateParameters();

        void checkUpdateLatency();

        void handleAsyncUpdate() override;
    };
}
