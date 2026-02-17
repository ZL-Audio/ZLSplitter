// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/analyzer/fft_analyzer/fft_analyzer_receiver.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_smoother.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_tilter.hpp"
#include "../../../dsp/analyzer/fft_analyzer/spectrum_decayer.hpp"
#include "../../../dsp/lock/spin_lock.hpp"
#include "../../../dsp/interpolation/seq_makima.hpp"

namespace zlpanel {
    class FFTAnalyzerPanel final : public juce::Component {
    public:
        explicit FFTAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~FFTAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void run();

        void resized() override;

        void setRefreshRate(double refresh_rate);

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        std::atomic<float> &split_type_ref_, &swap_ref_, &fft_min_db_ref_;

        AtomicBound<float> atomic_bound_;

        kfr::univector<float> xs_{}, y1s_{}, y2s_{};
        juce::Path out_path1_, next_out_path1_;
        juce::Path out_path2_, next_out_path2_;
        zldsp::lock::SpinLock mutex_;

        double c_sample_rate_{};
        int fft_size_{0};
        float c_width_{};

        float c_fft_min_db_{0.f};

        std::atomic<float> refresh_rate_{30.f};
        std::atomic<float> spectrum_decay_speed_{.375f};
        std::atomic<bool> to_update_decay_{false};

        std::atomic<float> spectrum_tilt_slope_{4.5f};
        std::atomic<bool> to_update_tilt_{false};

        std::atomic<bool> is_fft_frozen_{false};

        zldsp::analyzer::FFTAnalyzerReceiver<2> receiver_;
        zldsp::analyzer::SpectrumSmoother spectrum_smoother_;
        zldsp::analyzer::SpectrumTilter spectrum_tilter_;
        std::array<zldsp::analyzer::SpectrumDecayer, 2> spectrum_decayers_;

        size_t inter_num_{0};
        kfr::univector<float> inter_xs_{}, inter_y1s_{}, inter_y2s_{};
        std::unique_ptr<zldsp::interpolation::SeqMakima<float>> interpolator1_, interpolator2_;
    };
}