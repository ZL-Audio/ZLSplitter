// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ps_panel.hpp"

namespace zlPanel {
    PSPanel::PSPanel(juce::AudioProcessorValueTreeState &parameter, zlInterface::UIBase &base)
        : uiBase(base),
          balanceS("Balance", base),
          attackS("Attack", base),
          holdS("Hold", base),
          smoothS("Smooth", base) {
        attach({&balanceS.getSlider(), &attackS.getSlider(), &holdS.getSlider(), &smoothS.getSlider()},
               {zlDSP::psBalance::ID, zlDSP::psAttack::ID, zlDSP::psHold::ID, zlDSP::psSmooth::ID},
               parameter, sliderAttachments);
        for (auto &s: {&balanceS, &attackS, &holdS, &smoothS}) {
            s->setFontScale(1.125f, 1.125f);
            addAndMakeVisible(s);
        }
    }

    void PSPanel::resized() {
        for (auto &s: {&balanceS, &attackS, &holdS, &smoothS}) {
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
            juce::GridItem(attackS).withArea(1, 1),
            juce::GridItem(balanceS).withArea(2, 1),
            juce::GridItem(holdS).withArea(3, 1),
            juce::GridItem(smoothS).withArea(4, 1),
        };

        const auto bound = getLocalBounds();
        grid.performLayout(bound.toNearestInt());
    }
} // zlPanel
