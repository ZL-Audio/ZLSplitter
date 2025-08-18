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

    void FFTPanel::mouseEnter(const juce::MouseEvent &event) {
        startTimer(3000);
    }

    void FFTPanel::mouseMove(const juce::MouseEvent &event) {
        stopTimer();
        p_ref_.getController().getFFTAnalyzer().setFrozen(0, false);
        p_ref_.getController().getFFTAnalyzer().setFrozen(1, false);
        startTimer(3000);
    }

    void FFTPanel::mouseExit(const juce::MouseEvent &event) {
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
