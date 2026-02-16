// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "fft_panel.hpp"

namespace zlpanel {
    FFTPanel::FFTPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        fft_min_freq_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTMinFreq::kID)),
        fft_max_freq_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTMaxFreq::kID)),
        fft_background_panel_(p, base),
        fft_analyzer_panel_(p, base) {
        addAndMakeVisible(fft_background_panel_);
        addAndMakeVisible(fft_analyzer_panel_);
    }

    FFTPanel::~FFTPanel() = default;

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
        const auto d1 = std::abs(c_fft_min_freq_index_ - fft_min_freq_ref_.load(std::memory_order::relaxed));
        const auto d2 = std::abs(c_fft_max_freq_index_ - fft_max_freq_ref_.load(std::memory_order::relaxed));
        const auto d3 = std::abs(c_sample_rate_ - p_ref_.getAtomicSampleRate());
        if (d1 > .1f || d2 > .1f || d3 > 0.1) {
            c_fft_min_freq_index_ = fft_min_freq_ref_.load(std::memory_order::relaxed);
            c_fft_max_freq_index_ = fft_max_freq_ref_.load(std::memory_order::relaxed);
            c_sample_rate_ = p_ref_.getAtomicSampleRate();

            const auto min_freq = zlstate::PFFTMinFreq::kFreqs[static_cast<size_t>(std::round(c_fft_min_freq_index_))];
            double max_freq = 0.0;
            if (c_fft_max_freq_index_ < .5f) {
                max_freq = 10000.0;
            } else if (c_fft_max_freq_index_ < 1.5f) {
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
            fft_background_panel_.setMinMaxFreq(min_freq, max_freq);
        }
        const auto new_split_type = split_type_ref_.load(std::memory_order::relaxed);
        if (std::abs(new_split_type - c_split_type_) > .1f) {
            c_split_type_ = new_split_type;
        }
    }

    void FFTPanel::mouseEnter(const juce::MouseEvent&) {
        startTimer(kHoverDurationMS);
    }

    void FFTPanel::mouseMove(const juce::MouseEvent&) {
        stopTimer();
        startTimer(kHoverDurationMS);
    }

    void FFTPanel::mouseExit(const juce::MouseEvent&) {
        stopTimer();
    }

    void FFTPanel::timerCallback() {
        stopTimer();
    }

    void FFTPanel::visibilityChanged() {
    }
}
