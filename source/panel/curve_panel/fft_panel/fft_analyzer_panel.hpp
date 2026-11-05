// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../state/ui_state.hpp"

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/analyzer/fft_analyzer/fft_analyzer_receiver.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_smoother.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_tilter.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_decayer.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_blender.hpp"
#include "../../../dsp/lock/spin_lock.hpp"
#include "../../../dsp/interpolation/seq_makima.hpp"

namespace zlpanel {
    class FFTAnalyzerPanel final : public juce::Component {
    public:
        explicit FFTAnalyzerPanel(PluginProcessor &p, zlgui::UIState &base);

        ~FFTAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void run();

        void reset() { c_sample_rate_ = 0.0; }

        void resized() override;

        void setRefreshRate(double refresh_rate);

    private:
        static constexpr size_t kNumSources = 2;
        static constexpr size_t kNumResolutions = 3;
        static constexpr size_t kLowResolution = 0;
        static constexpr size_t kMiddleResolution = 1;
        static constexpr size_t kHighResolution = 2;
        static constexpr size_t kInterSize = 64;

        PluginProcessor &p_ref_;
        zlgui::UIState &base_;
        std::atomic<float> &split_type_ref_, &swap_ref_, &fft_min_db_ref_;

        std::atomic<float> &fft_smooth_oct_value_idx_ref_;
        std::atomic<float> &fft_smooth_erb_value_idx_ref_;
        std::atomic<float> &fft_smooth_type_idx_ref_;
        int fft_smooth_oct_value_idx_{-1};
        int fft_smooth_erb_value_idx_{-1};
        int fft_smooth_type_idx_{-1};

        std::atomic<float> &fft_speed_idx_ref_;
        int fft_speed_idx_{zlstate::PFFTSpeed::kDefaultI};

        std::atomic<float> &fft_tilt_idx_ref_;
        int fft_tilt_idx_{zlstate::PFFTTilt::kDefaultI};

        std::atomic<float> fft_extra_tilt_{0.f};
        std::atomic<float> fft_extra_speed_{1.f};

        AtomicBound<float> atomic_bound_;
        uint64_t generation_{};

        zldsp::vector::aligned_vector<float> xs_{}, ys_{};
        std::vector<float> frequencies_{};
        juce::Path out_path1_, next_out_path1_;
        juce::Path out_path2_, next_out_path2_;
        zldsp::lock::SpinLock mutex_;

        double c_sample_rate_{};
        bool c_high_quality_{false};
        int history_size_{0};
        float c_width_{};

        float c_fft_min_db_{0.f};

        std::atomic<float> refresh_rate_{30.f};
        std::atomic<bool> to_update_decay_{false};

        std::atomic<float> spectrum_tilt_slope_{4.5f};
        std::atomic<bool> to_update_tilt_{false};

        std::atomic<bool> is_fft_frozen_{false};

        std::array<zldsp::analyzer::FFTAnalyzerProcessor, kNumResolutions> processors_;
        std::array<zldsp::analyzer::FFTAnalyzerReceiver, kNumSources> receivers_{
            zldsp::analyzer::FFTAnalyzerReceiver{processors_[kMiddleResolution]},
            zldsp::analyzer::FFTAnalyzerReceiver{processors_[kMiddleResolution]}
        };
        std::array<zldsp::analyzer::SpectrumSmoother, kNumResolutions> smoothers_;
        zldsp::analyzer::SpectrumTilter spectrum_tilter_;
        std::array<zldsp::analyzer::SpectrumDecayer, kNumSources> spectrum_decayers_;
        std::array<std::array<zldsp::vector::aligned_vector<float>, kNumResolutions>,
                   kNumSources> resolution_spectra_;
        std::array<zldsp::vector::aligned_vector<float>, kNumSources> spectra_;
        std::array<float, kNumResolutions> noise_power_scales_{};

        std::array<float, kInterSize + 2> inter_xs_{}, inter_ys_{};
        std::unique_ptr<zldsp::interpolation::SeqMakima<float>> inter_;

        void lookAndFeelChanged() override;
    };
}
