// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "fft_analyzer_panel.hpp"
#include "fft_background_panel.hpp"

namespace zlpanel {
    class FFTPanel final : public juce::Component,
                           private juce::Timer {
    public:
        explicit FFTPanel(PluginProcessor &processor, zlgui::UIBase &base);

        void run();

        void resized() override;

    private:
        PluginProcessor &p_ref_;
        FFTBackgroundPanel fft_background_panel_;
        FFTAnalyzerPanel fft_analyzer_panel_;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void timerCallback() override;

        void visibilityChanged() override;
    };
}
