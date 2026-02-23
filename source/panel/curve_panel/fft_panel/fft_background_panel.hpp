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
#include "../../helper/helper.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class FFTBackgroundPanel final : public juce::Component {
    public:
        explicit FFTBackgroundPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics& g) override;

        void updateSampleRate(double sample_rate);

        void repaintCallBackSlow();

    private:
        static constexpr std::array kFreqValues = {
            20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f,
            10000.f, 20000.f, 50000.f, 100000.f, 200000.f
        };

        zlgui::UIBase& base_;
        std::atomic<float>& fft_min_db_;
        float c_fft_min_db_{-1.f};
        double freq_max_{0.};

        juce::Colour grid_colour_;

        void drawFreqs(juce::Graphics& g) const;

        void drawDBs(juce::Graphics& g) const;

        void lookAndFeelChanged() override;
    };
}