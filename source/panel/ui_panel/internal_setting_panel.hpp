// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef INTERNAL_SETTING_PANEL_HPP
#define INTERNAL_SETTING_PANEL_HPP

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"

namespace zlPanel {
    class InternalSettingPanel final : public juce::Component {
    public:
        static constexpr float heightP = 28.f;

        explicit InternalSettingPanel(PluginProcessor &p, zlInterface::UIBase &base);

        void loadSetting();

        void saveSetting();

        void resetSetting();

        void mouseDown(const juce::MouseEvent &event) override;

        void resized() override;

    private:
        inline static constexpr float iHeight = 5.f;
        PluginProcessor &pRef;
        zlInterface::UIBase &uiBase;
        zlInterface::NameLookAndFeel nameLAF;
        zlInterface::ColourOpacitySelector textSelector, backgroundSelector, shadowSelector, glowSelector;
        juce::Label colourImportLabel, controlImportLabel;

        static constexpr size_t numSelectors = 4;
        std::array<juce::Label, numSelectors> selectorLabels;
        std::array<zlInterface::ColourOpacitySelector*, numSelectors> selectors{
            &textSelector,
            &backgroundSelector,
            &shadowSelector,
            &glowSelector
        };
        std::array<std::string, numSelectors> selectorNames{
            "Text Colour",
            "Background Colour",
            "Shadow Colour",
            "Glow Colour"
        };

        std::array<zlInterface::colourIdx, numSelectors> colourIdx {
            zlInterface::colourIdx::textColour,
            zlInterface::colourIdx::backgroundColour,
            zlInterface::colourIdx::shadowColour,
            zlInterface::colourIdx::glowColour
        };

        std::array<std::string, numSelectors> colourTagNames{
            "text_colour",
            "background_colour",
            "shadow_colour",
            "glow_colour",
        };

        std::unique_ptr<juce::FileChooser> myChooser;
        inline auto static const settingDirectory =
                juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile("Audio")
                .getChildFile("Presets")
                .getChildFile(JucePlugin_Manufacturer)
                .getChildFile("Shared Settings");

        void importColours();

        void importControls();
    };
} // zlPanel

#endif //INTERNAL_SETTING_PANEL_HPP
