// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_panel.hpp"
#include "BinaryData.h"

namespace zlPanel {
    UISettingPanel::UISettingPanel(PluginProcessor &p, zlInterface::UIBase &base)
        : pRef(p), uiBase(base),
          internelPanel(p, uiBase),
          saveDrawable(juce::Drawable::createFromImageData(BinaryData::saveline_svg, BinaryData::saveline_svgSize)),
          closeDrawable(juce::Drawable::createFromImageData(BinaryData::xmark_svg, BinaryData::xmark_svgSize)),
          resetDrawable(
              juce::Drawable::createFromImageData(BinaryData::loopleftline_svg, BinaryData::loopleftline_svgSize)),
          saveButton(uiBase, saveDrawable.get()),
          closeButton(uiBase, closeDrawable.get()),
          resetButton(uiBase, resetDrawable.get()),
          labelLAF(uiBase) {
        juce::ignoreUnused(pRef);
        setOpaque(true);
        addAndMakeVisible(saveButton);
        addAndMakeVisible(closeButton);
        addAndMakeVisible(resetButton);
        viewPort.setScrollBarsShown(true, false,
                                    true, false);
        viewPort.setViewedComponent(&internelPanel, false);
        addAndMakeVisible(viewPort);
        saveButton.getButton().onClick = [this]() {
            internelPanel.saveSetting();
        };
        resetButton.getButton().onClick = [this]() {
            internelPanel.resetSetting();
        };
        closeButton.getButton().onClick = [this]() {
            setVisible(false);
        };

        labelLAF.setFontScale(.85f);
        labelLAF.setAlpha(.5f);
        labelLAF.setJustification(juce::Justification::centred);
        versionLabel.setText(
            juce::String(ZLEQUALIZER_CURRENT_VERSION) + " " + juce::String(ZLEQUALIZER_CURRENT_HASH),
            juce::dontSendNotification);
        versionLabel.setLookAndFeel(&labelLAF);
        addAndMakeVisible(versionLabel);
    }

    UISettingPanel::~UISettingPanel() {
        versionLabel.setLookAndFeel(nullptr);
    }

    void UISettingPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        auto bound = getLocalBounds().toFloat();
        bound.removeFromTop(uiBase.getFontSize());
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .95f, bound.getHeight());
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.5f});
    }

    void UISettingPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        versionLabel.setBounds(bound.removeFromTop(uiBase.getFontSize()).toNearestInt());
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .925f, bound.getHeight());
        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});

        internelPanel.setBounds(0, 0,
                                juce::roundToInt(bound.getWidth()),
                                juce::roundToInt((InternalSettingPanel::heightP + 1.f) * uiBase.getFontSize()));
        viewPort.setBounds(bound.removeFromTop(bound.getHeight() * .9f).toNearestInt());
        const auto iconWidth = bound.getHeight() * .95f;
        const auto leftBound = bound.removeFromLeft(
            bound.getWidth() * .3333333f).withSizeKeepingCentre(iconWidth, iconWidth);
        const auto centerBound = bound.removeFromLeft(
            bound.getWidth() * .5f).withSizeKeepingCentre(iconWidth, iconWidth);
        const auto rightBound = bound.withSizeKeepingCentre(iconWidth, iconWidth);
        saveButton.setBounds(leftBound.toNearestInt());
        resetButton.setBounds(centerBound.toNearestInt());
        closeButton.setBounds(rightBound.toNearestInt());
    }

    void UISettingPanel::visibilityChanged() {
        internelPanel.loadSetting();
    }
} // zlPanel
