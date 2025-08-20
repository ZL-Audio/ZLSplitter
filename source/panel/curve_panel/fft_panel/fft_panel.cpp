// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 8/18/25.
//

#include "fft_panel.hpp"

namespace zlpanel {
    FFTPanel::FFTPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : p_ref_(processor),
          fft_min_freq_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PFFTMinFreq::kID)),
          fft_max_freq_ref_(*processor.na_parameters_.getRawParameterValue(zlstate::PFFTMaxFreq::kID)),
          fft_background_panel_(processor, base),
          fft_analyzer_panel_(processor, base) {
        addAndMakeVisible(fft_background_panel_);
        addAndMakeVisible(fft_analyzer_panel_);
    }

    void FFTPanel::run() {
        fft_analyzer_panel_.run();
    }

    void FFTPanel::resized() {
        const auto bound = getLocalBounds();
        fft_background_panel_.setBounds(bound);
        fft_analyzer_panel_.setBounds(bound);
    }

    void FFTPanel::repaintCallBackSlow() {
        fft_background_panel_.repaintCallBackSlow();
        const auto d1 = std::abs(c_fft_min_freq_index - fft_min_freq_ref_.load(std::memory_order::relaxed));
        const auto d2 = std::abs(c_fft_max_freq_index - fft_max_freq_ref_.load(std::memory_order::relaxed));
        const auto d3 = std::abs(c_sample_rate_ - p_ref_.getAtomicSampleRate());
        if (d1 > .1f || d2 > .1f || d3 > 0.1) {
            c_fft_min_freq_index = fft_min_freq_ref_.load(std::memory_order::relaxed);
            c_fft_max_freq_index = fft_max_freq_ref_.load(std::memory_order::relaxed);
            c_sample_rate_ = p_ref_.getAtomicSampleRate();

            const auto min_freq = zlstate::PFFTMinFreq::kFreqs[static_cast<size_t>(std::round(c_fft_min_freq_index))];
            double max_freq = 0.0;
            if (c_fft_max_freq_index < .5f) {
                max_freq = 10000.0;
            } else if (c_fft_max_freq_index < 1.5f) {
                max_freq = 20000.0;
            } else {
                if (c_sample_rate_ < 45000.0) {
                    max_freq = 22000.0;
                } else if (c_sample_rate_ < 50000.0) {
                    max_freq = 24000.0;
                } else if (c_sample_rate_ < 90000.0) {
                    max_freq = 44000.0;
                } else if (c_sample_rate_ < 100000.0) {
                    max_freq = 48000.0;
                } else if (c_sample_rate_ < 135000.0) {
                    max_freq = 66000.0;
                } else if (c_sample_rate_ < 150000.0) {
                    max_freq = 72000.0;
                } else if (c_sample_rate_ < 180000.0) {
                    max_freq = 88000.0;
                } else {
                    max_freq = 96000.0;
                }
            }
            p_ref_.getController().getFFTAnalyzer().setMinFreq(min_freq);
            p_ref_.getController().getFFTAnalyzer().setMaxFreq(max_freq);
            fft_background_panel_.setMinMaxFreq(min_freq, max_freq);
        }
    }

    void FFTPanel::mouseEnter(const juce::MouseEvent &) {
        startTimer(kHoverDurationMS);
    }

    void FFTPanel::mouseMove(const juce::MouseEvent &) {
        stopTimer();
        p_ref_.getController().getFFTAnalyzer().setFrozen(0, false);
        p_ref_.getController().getFFTAnalyzer().setFrozen(1, false);
        startTimer(kHoverDurationMS);
    }

    void FFTPanel::mouseExit(const juce::MouseEvent &) {
        stopTimer();
        p_ref_.getController().getFFTAnalyzer().setFrozen(0, false);
        p_ref_.getController().getFFTAnalyzer().setFrozen(1, false);
    }

    void FFTPanel::timerCallback() {
        p_ref_.getController().getFFTAnalyzer().setFrozen(0, true);
        p_ref_.getController().getFFTAnalyzer().setFrozen(1, true);
        stopTimer();
    }

    void FFTPanel::visibilityChanged() {
        auto &analyzer{p_ref_.getController().getFFTAnalyzer()};
        if (isVisible()) {
            analyzer.setON({true, true});
        } else {
            analyzer.setON({false, false});
        }
    }
}
