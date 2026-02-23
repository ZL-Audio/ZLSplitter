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
        fft_background_panel_(p, base),
        fft_analyzer_panel_(p, base) {
        juce::ignoreUnused(p_ref_);
        fft_background_panel_.setBufferedToImage(true);
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
        fft_background_panel_.updateSampleRate(p_ref_.getAtomicSampleRate());
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
