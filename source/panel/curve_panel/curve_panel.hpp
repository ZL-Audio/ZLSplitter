// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../multilingual/tooltip_helper.hpp"

#include "fft_panel/fft_panel.hpp"
#include "mag_panel/mag_panel.hpp"
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

        void setRefreshRate(const double refresh_rate) {
            fft_panel_.setRefreshRate(refresh_rate);
        }

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        std::atomic<float> &split_type_ref_;
        std::atomic<float> &analyzer_show_ref_;
        std::atomic<double> next_time_stamp_{0.};

        FFTPanel fft_panel_;
        MagPanel mag_panel_;
        LeftControlPanel left_control_panel_;
        LeftPopPanel left_pop_panel_;

        void run() override;
    };
} // zlpanel