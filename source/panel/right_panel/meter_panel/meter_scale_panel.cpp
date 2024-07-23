// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "meter_scale_panel.hpp"

namespace zlPanel {
    MeterScalePanel::MeterScalePanel(zlInterface::UIBase &base)
        : uiBase(base) {
        setBufferedToImage(true);
        setInterceptsMouseClicks(false, false);
    }

    void MeterScalePanel::paint(juce::Graphics &g) {
        auto bound = getLocalBounds().toFloat();
        bound.removeFromTop(bound.getHeight() * labelPortion);
        auto startDB = juce::roundToInt(maxDB);
        const auto intervalDB = juce::roundToInt((maxDB - minDB) / static_cast<float>(numScales));
        g.setFont(uiBase.getFontSize() * 1.125f);
        const auto thickness = uiBase.getFontSize() * .125f;
        for (size_t i = 0; i < numScales; ++i) {
            const auto portion = (static_cast<float>(startDB) - minDB) / (maxDB - minDB);
            const auto y = bound.getY() + (1 - portion) * bound.getHeight();
            const auto fontBound = juce::Rectangle<float>(bound.getX(), y - uiBase.getFontSize(),
                bound.getWidth(), 2 * uiBase.getFontSize());
            if (!ignoreFirst || i != 0) {
                g.setColour(uiBase.getTextColor());
                g.drawText(juce::String(-startDB), fontBound, juce::Justification::centred);
                g.drawLine(fontBound.getCentreX() - .55f * uiBase.getFontSize(), y,
                    fontBound.getCentreX() - .875f * uiBase.getFontSize(), y,
                    thickness);
                g.drawLine(fontBound.getCentreX() + .55f * uiBase.getFontSize(), y,
                    fontBound.getCentreX() + .875f * uiBase.getFontSize(), y,
                    thickness);
            } else {
                g.setColour(uiBase.getTextInactiveColor());
                g.drawLine(fontBound.getCentreX() - .875f * uiBase.getFontSize(), y,
                    fontBound.getCentreX() + .875f * uiBase.getFontSize(), y,
                    thickness * .5f);
            }
            startDB -= intervalDB;
        }
    }
} // zlPanel
