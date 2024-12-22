// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &processor)
        : uiBase(processor.state),
          uiPanel(processor, uiBase),
          topPanel(processor, uiBase, uiPanel),
          controlPanel(processor, uiBase),
          meterPanel(processor, uiBase) {
        uiBase.loadFromAPVTS();
        addAndMakeVisible(topPanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(meterPanel);
        addChildComponent(uiPanel);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        uiBase.setFontSize(bound.getWidth() * 0.06767659574468085f);

        uiPanel.setBounds(getLocalBounds());

        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});

        topPanel.setBounds(bound.removeFromTop(bound.getHeight() * .2f).toNearestInt());
        controlPanel.setBounds(bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        meterPanel.setBounds(bound.toNearestInt());
    }
} // zlPanel
