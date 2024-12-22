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
    InternalSettingPanel::InternalSettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p),
          uiBase(base), nameLAF(base),
          textSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          backgroundSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          shadowSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          glowSelector(base, *this, false, 9.6f, 8.f, .5f, .4f),
          roughWheelSlider("Rough", base),
          fineWheelSlider("Fine", base),
          rotaryStyleBox(zlState::rotaryStyle::choices, base),
          rotaryDragSensitivitySlider("Distance", base) {
        juce::ignoreUnused(pRef);
        nameLAF.setJustification(juce::Justification::centred);
        nameLAF.setFontScale(1.f);
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setText(selectorNames[i], juce::dontSendNotification);
            selectorLabels[i].setLookAndFeel(&nameLAF);
            addAndMakeVisible(selectorLabels[i]);
            addAndMakeVisible(selectors[i]);
        }
        wheelLabel.setText("Mouse-Wheel Sensitivity", juce::dontSendNotification);
        wheelLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(wheelLabel);
        roughWheelSlider.getSlider().setDoubleClickReturnValue(true, 1.0);
        fineWheelSlider.getSlider().setDoubleClickReturnValue(true, 0.1);
        for (auto &s: {&roughWheelSlider, &fineWheelSlider}) {
            s->getSlider().setRange(0.0, 1.0, 0.01);
            s->setFontScale(1.125f, 1.125f);
            addAndMakeVisible(s);
        }
        rotaryStyleLabel.setText("Rotary Slider Style", juce::dontSendNotification);
        rotaryStyleLabel.setLookAndFeel(&nameLAF);
        addAndMakeVisible(rotaryStyleLabel);
        rotaryStyleBox.getLAF().setFontScale(1.125f);
        addAndMakeVisible(rotaryStyleBox);
        rotaryDragSensitivitySlider.setFontScale(1.125f, 1.125f);
        rotaryDragSensitivitySlider.getSlider().setRange(2.0, 32.0, 0.01);
        rotaryDragSensitivitySlider.getSlider().setDoubleClickReturnValue(true, 10.0);
        addAndMakeVisible(rotaryDragSensitivitySlider);
    }

    InternalSettingPanel::~InternalSettingPanel() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectorLabels[i].setLookAndFeel(nullptr);
        }
        wheelLabel.setLookAndFeel(nullptr);
        rotaryStyleLabel.setLookAndFeel(nullptr);
    }

    void InternalSettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        for (size_t i = 0; i < numSelectors; ++i) {
            bound.removeFromTop(uiBase.getFontSize() * .5f);
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * iHeight);
            selectorLabels[i].setBounds(localBound.removeFromTop(localBound.getHeight() * .5f).toNearestInt());
            selectors[i]->setBounds(localBound.toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize() * .5f);
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * iHeight);
            wheelLabel.setBounds(localBound.removeFromTop(localBound.getHeight() * .5f).toNearestInt());
            const auto sWidth = (bound.getWidth() - uiBase.getFontSize() * 1.f) * 0.45f;
            roughWheelSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 1.f);
            fineWheelSlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        } {
            bound.removeFromTop(uiBase.getFontSize() * .5f);
            auto localBound = bound.removeFromTop(uiBase.getFontSize() * iHeight);
            rotaryStyleLabel.setBounds(localBound.removeFromTop(localBound.getHeight() * .5f).toNearestInt());
            const auto sWidth = (bound.getWidth() - uiBase.getFontSize() * 1.f) * 0.45f;
            rotaryStyleBox.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
            localBound.removeFromLeft(uiBase.getFontSize() * 1.f);
            rotaryDragSensitivitySlider.setBounds(localBound.removeFromLeft(sWidth).toNearestInt());
        }
    }

    void InternalSettingPanel::loadSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            selectors[i]->setColour(uiBase.getColourByIdx(static_cast<zlInterface::colourIdx>(i)));
        }
        roughWheelSlider.getSlider().setValue(static_cast<double>(uiBase.getWheelSensitivity(0)));
        fineWheelSlider.getSlider().setValue(static_cast<double>(uiBase.getWheelSensitivity(1)));
        rotaryStyleBox.getBox().setSelectedId(static_cast<int>(uiBase.getRotaryStyleID()) + 1);
        rotaryDragSensitivitySlider.getSlider().setValue(static_cast<double>(uiBase.getRotaryDragSensitivity()));
    }

    void InternalSettingPanel::saveSetting() {
        for (size_t i = 0; i < numSelectors; ++i) {
            uiBase.setColourByIdx(static_cast<zlInterface::colourIdx>(i), selectors[i]->getColour());
        }
        uiBase.setWheelSensitivity(static_cast<float>(roughWheelSlider.getSlider().getValue()), 0);
        uiBase.setWheelSensitivity(static_cast<float>(fineWheelSlider.getSlider().getValue()), 1);
        uiBase.setRotaryStyleID(static_cast<size_t>(rotaryStyleBox.getBox().getSelectedId() - 1));
        uiBase.setRotaryDragSensitivity(static_cast<float>(rotaryDragSensitivitySlider.getSlider().getValue()));
        uiBase.saveToAPVTS();
    }

    void InternalSettingPanel::resetSetting() {
        loadSetting();
    }
} // zlPanel
