// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef LOGO_PANEL_HPP
#define LOGO_PANEL_HPP

#include "../../state/state.hpp"
#include "../../gui/gui.hpp"
#include "../ui_panel/ui_setting_panel.hpp"

namespace zlPanel {
    class LogoPanel final : public juce::Component {
    public:
        explicit LogoPanel(PluginProcessor &processor,
                           zlInterface::UIBase &base,
                           UISettingPanel &panelToShow);

        void paint(juce::Graphics &g) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void setJustification(int justificationFlags);

    private:
        juce::AudioProcessorValueTreeState &stateRef;
        zlInterface::UIBase &uiBase;
        UISettingPanel &uiPanel;
        const std::unique_ptr<juce::Drawable> brandDrawable, logoDrawable;
        juce::Justification justification {juce::Justification::topLeft};
    };
} // zlPanel

#endif //ZLEqualizer_LOGO_PANEL_HPP
