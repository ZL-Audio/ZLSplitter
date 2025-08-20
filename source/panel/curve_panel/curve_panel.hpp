// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../multilingual/tooltip_helper.hpp"

#include "fft_panel/fft_panel.hpp"
#include "left_control_panel/left_control_panel.hpp"
#include "left_pop_panel/left_pop_panel.hpp"

namespace zlpanel {
    class CurvePanel final : public juce::Component,
                             private juce::Thread {
    public:
        explicit CurvePanel(PluginProcessor &p, zlgui::UIBase &base,
                            multilingual::TooltipHelper &tooltip_helper);

        ~CurvePanel() override;

        void paint(juce::Graphics &g) override;

        void paintOverChildren(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBack(double time_stamp);

        void repaintCallBackSlow();

        void mouseDown(const juce::MouseEvent &event) override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;

        FFTPanel fft_panel_;
        LeftControlPanel left_control_panel_;
        LeftPopPanel left_pop_panel_;

        void run() override;
    };
} // zlpanel
