// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef TOP_PANEL_HPP
#define TOP_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../panel_definitons.hpp"

#include "logo_panel.hpp"

namespace zlPanel {
    class TopPanel final : public juce::Component {
    public:
        explicit TopPanel(PluginProcessor &processor,
                          zlInterface::UIBase &base,
                          UISettingPanel &panelToShow);

        void resized() override;

    private:
        LogoPanel logoPanel;

        const std::unique_ptr<juce::Drawable> swapIcon, lrIcon, msIcon, lhIcon, tsIcon;

        zlInterface::CompactButton swapButton;
        zlInterface::CompactFigureCombobox splitBox;

        juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAttachments;
        zlInterface::ComboBoxSubAttachment boxAttachment;
    };
} // zlPanel

#endif //TOP_PANEL_HPP
