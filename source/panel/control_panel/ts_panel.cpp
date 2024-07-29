// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ts_panel.hpp"

namespace zlPanel {
    TSPanel::TSPanel(juce::AudioProcessorValueTreeState &parameter, zlInterface::UIBase &base)
        : uiBase(base),
          separationS("Strength", base),
          balanceS("Balance", base),
          holdS("Hold", base),
          smoothS("Smooth", base) {
        attach({&separationS.getSlider(), &balanceS.getSlider(), &holdS.getSlider(), &smoothS.getSlider()},
               {zlDSP::tsSeperation::ID, zlDSP::tsBalance::ID, zlDSP::tsHold::ID, zlDSP::tsSmooth::ID},
               parameter, sliderAttachments);
        for (auto &s: {&separationS, &balanceS, &holdS, &smoothS}) {
            s->setFontScale(1.125f, 1.125f);
            addAndMakeVisible(s);
        }
    }

    void TSPanel::resized() {
        for (auto &s: {&separationS, &balanceS, &holdS, &smoothS}) {
            s->setPadding(uiBase.getFontSize() * 1.f, uiBase.getFontSize() * .5f);
        }

        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {
            Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1))
        };
        grid.templateColumns = {
            Track(Fr(1))
        };
        grid.items = {
            juce::GridItem(separationS).withArea(1, 1),
            juce::GridItem(balanceS).withArea(2, 1),
            juce::GridItem(holdS).withArea(3, 1),
            juce::GridItem(smoothS).withArea(4, 1),
        };

        const auto bound = getLocalBounds();
        // bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());
    }
} // zlPanel
