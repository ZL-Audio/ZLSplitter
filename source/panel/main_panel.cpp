// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlPanel {
    MainPanel::MainPanel(PluginProcessor &processor)
        : uiBase(processor.state),
          topPanel(processor, uiBase),
          controlPanel(processor, uiBase),
          meterPanel(processor, uiBase) {
        uiBase.loadFromAPVTS();
        addAndMakeVisible(topPanel);
        addAndMakeVisible(controlPanel);
        addAndMakeVisible(meterPanel);
    }

    void MainPanel::paint(juce::Graphics &g) {
        g.fillAll(uiBase.getBackgroundColor());
        const auto bound = getLocalBounds().toFloat();
        uiBase.fillRoundedShadowRectangle(g, bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        uiBase.setFontSize(bound.getWidth() * 0.06767659574468085f);

        bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {.blurRadius = 0.25f});

        topPanel.setBounds(bound.removeFromTop(bound.getHeight() * .2f).toNearestInt());
        controlPanel.setBounds(bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        meterPanel.setBounds(bound.toNearestInt());
    }
} // zlPanel
