// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef MAIN_PANEL_HPP
#define MAIN_PANEL_HPP

#include "right_panel/meter_panel.hpp"
#include "top_panel/top_panel.hpp"
#include "control_panel/control_panel.hpp"

namespace zlPanel {
    class MainPanel final : public juce::Component {
    public:
        explicit MainPanel(PluginProcessor &processor);

        void paint(juce::Graphics &g) override;

        void resized() override;

    private:
        zlInterface::UIBase uiBase;
        UISettingPanel uiPanel;
        TopPanel topPanel;
        ControlPanel controlPanel;
        MeterPanel meterPanel;
    };
} // zlPanel

#endif //MAIN_PANEL_HPP
