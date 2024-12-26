// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "internal_setting_panel.hpp"

namespace zlPanel {
    static juce::Colour getIntColour(const int r, const int g, const int b, float alpha) {
        return {
            static_cast<juce::uint8>(r),
            static_cast<juce::uint8>(g),
            static_cast<juce::uint8>(b),
            alpha
        };
    }

    InternalSettingPanel::InternalSettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p),
          uiBase(base), nameLAF(base),
          textSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          backgroundSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          shadowSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          glowSelector(base, *this, false, 9.6f, 8.f, .5f, .4f) {
        juce::ignoreUnused(pRef);
        nameLAF.setJustification(juce::Justification::centred);
        nameLAF.setFontScale(1.f);
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setText(selectorNames[i], juce::dontSendNotification);
            selectorLabels[i].setLookAndFeel(&nameLAF);
            addAndMakeVisible(selectorLabels[i]);
            addAndMakeVisible(selectors[i]);
        }
        colourImportLabel.setText("Import Colours", juce::dontSendNotification);
        colourImportLabel.setLookAndFeel(&nameLAF);
        colourImportLabel.addMouseListener(this, false);
        addAndMakeVisible(colourImportLabel);
        controlImportLabel.setText("Import Controls", juce::dontSendNotification);
        controlImportLabel.setLookAndFeel(&nameLAF);
        controlImportLabel.addMouseListener(this, false);
        addAndMakeVisible(controlImportLabel);
    }

    void InternalSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        for (size_t i = 0; i < numSelectors; ++i) {
            bound.removeFromTop(uiBase.getFontSize() * .5f);
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * iHeight);
            selectorLabels[i].setBounds(localBound.removeFromTop(localBound.getHeight() * .5f).toNearestInt());
            selectors[i]->setBounds(localBound.toNearestInt());
        }
        {
            bound.removeFromTop(uiBase.getFontSize() * .5f);
            const auto localBound = bound.removeFromTop(uiBase.getFontSize() * iHeight * .5f);
            colourImportLabel.setBounds(localBound.toNearestInt());
        }
        {
            bound.removeFromTop(uiBase.getFontSize() * .5f);
            const auto localBound = bound.removeFromTop(uiBase.getFontSize() * iHeight * .5f);
            controlImportLabel.setBounds(localBound.toNearestInt());
        }
    }

    void InternalSettingPanel::loadSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectors[i]->setColour(uiBase.getColourByIdx(colourIdx[i]));
        }
    }

    void InternalSettingPanel::saveSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            uiBase.setColourByIdx(colourIdx[i], selectors[i]->getColour());
        }
        uiBase.saveToAPVTS();
    }

    void InternalSettingPanel::resetSetting() {
        textSelector.setColour(getIntColour(247, 246, 244, 1.f));
        backgroundSelector.setColour(getIntColour((255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, 1.f));
        shadowSelector.setColour(getIntColour(0, 0, 0, 1.f));
        glowSelector.setColour(getIntColour(70, 66, 62, 1.f));
        saveSetting();
    }

    void InternalSettingPanel::mouseDown(const juce::MouseEvent &event) {
        if (event.originalComponent == &colourImportLabel) {
            importColours();
        } else if (event.originalComponent == &controlImportLabel) {
            importControls();
        }
    }

    void InternalSettingPanel::importColours() {
        myChooser = std::make_unique<juce::FileChooser>(
                "Load the colour settings...", settingDirectory, "*.xml",
                true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles;
        myChooser->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File settingFile(chooser.getResult());
            if (const auto xmlInput = juce::XmlDocument::parse(settingFile)) {
                for (size_t i = 0; i < numSelectors; ++i) {
                    if (const auto *xmlColour = xmlInput->getChildByName(colourTagNames[i])) {
                        const juce::Colour colour = getIntColour(
                            xmlColour->getIntAttribute("r"),
                            xmlColour->getIntAttribute("g"),
                            xmlColour->getIntAttribute("b"),
                            static_cast<float>(xmlColour->getDoubleAttribute("o")));
                        uiBase.setColourByIdx(colourIdx[i], colour);
                    }
                }
                uiBase.saveToAPVTS();
                loadSetting();
            }
        });
    }

    void InternalSettingPanel::importControls() {
        myChooser = std::make_unique<juce::FileChooser>(
                "Load the control settings...", settingDirectory, "*.xml",
                true, false, nullptr);
        constexpr auto settingOpenFlags = juce::FileBrowserComponent::openMode |
                                          juce::FileBrowserComponent::canSelectFiles;
        myChooser->launchAsync(settingOpenFlags, [this](const juce::FileChooser &chooser) {
            if (chooser.getResults().size() <= 0) { return; }
            const juce::File settingFile(chooser.getResult());
            if (const auto xmlInput = juce::XmlDocument::parse(settingFile)) {
                if (const auto *xmlElement = xmlInput->getChildByName("drag_fine_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setSensitivity(static_cast<float>(x), zlInterface::sensitivityIdx::mouseDragFine);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("drag_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setSensitivity(static_cast<float>(x), zlInterface::sensitivityIdx::mouseDrag);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("wheel_fine_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setSensitivity(static_cast<float>(x), zlInterface::sensitivityIdx::mouseWheelFine);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("wheel_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setSensitivity(static_cast<float>(x), zlInterface::sensitivityIdx::mouseWheel);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("rotary_drag_sensitivity")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setRotaryDragSensitivity(static_cast<float>(x));
                }
                if (const auto *xmlElement = xmlInput->getChildByName("rotary_style")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setRotaryStyleID(static_cast<size_t>(x));
                }
                if (const auto *xmlElement = xmlInput->getChildByName("slider_double_click_func")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setIsSliderDoubleClickOpenEditor(x > 0.5);
                }
                if (const auto *xmlElement = xmlInput->getChildByName("wheel_shift_reverse")) {
                    const auto x = xmlElement->getDoubleAttribute("value");
                    uiBase.setIsMouseWheelShiftReverse(x > 0.5);
                }
                uiBase.saveToAPVTS();
                loadSetting();
            }
        });
    }
} // zlPanel
