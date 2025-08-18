// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../PluginProcessor.hpp"
#include "../gui/gui.hpp"
#include "../state/state_definitions.hpp"
#include "helper/refresh_handler.hpp"
#include "multilingual/tooltip_helper.hpp"
#include "curve_panel/curve_panel.hpp"
#include "ui_setting_panel/ui_setting_panel.hpp"

namespace zlpanel {
    class MainPanel final : public juce::Component,
                            private juce::ValueTree::Listener,
                            private juce::Timer {
    public:
        explicit MainPanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~MainPanel() override;

        void resized() override;

        void repaintCallBack(double time_stamp);

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        multilingual::TooltipHelper tooltip_helper_;

        CurvePanel curve_panel_;
        UISettingPanel ui_setting_panel_;

        zlgui::tooltip::TooltipLookAndFeel tooltip_laf_;
        zlgui::tooltip::TooltipWindow tooltip_window_;

        RefreshHandler refresh_handler_;
        double previous_time_stamp_{-1.0};
        double refresh_rate_{-1.0};

        void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &property) override;

        void timerCallback() override;
    };
} // zlpanel
