// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "lh_panel.hpp"

#include "lr_panel.hpp"

namespace zlPanel {
    LHPanel::LHPanel(juce::AudioProcessorValueTreeState &parameter, zlInterface::UIBase &base)
        : uiBase(base),
          mixS("Mix", base),
          freqS("Freq", base),
          ftypeC("", zlDSP::lhFilterType::choices, base),
          slopeC("", zlDSP::lhSlope::choices, base) {
        freqS.setShowSlider2(false);
        attach({&mixS.getSlider(), &freqS.getSlider1()},
               {zlDSP::mix::ID, zlDSP::lhFreq::ID},
               parameter, sliderAttachments);
        attach({&ftypeC.getBox(), &slopeC.getBox()},
            {zlDSP::lhFilterType::ID, zlDSP::lhSlope::ID},
            parameter, boxAttachments);
        mixS.setFontScale(1.125f, 1.125f);
        addAndMakeVisible(mixS);
        for (auto &c : {&ftypeC, &slopeC}) {
            c->getLAF().setFontScale(1.125f);
            addAndMakeVisible(c);
        }
        freqS.setFontScale(1.25f, 1.25f);
        addAndMakeVisible(freqS);
    }

    void LHPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        const auto height = bound.getHeight() * .25f;
        mixS.setPadding(uiBase.getFontSize() * 1.f, uiBase.getFontSize() * .5f);
        mixS.setBounds(bound.removeFromTop(height).toNearestInt());
        auto boxBound = bound.removeFromTop(height);
        auto leftBound = boxBound.removeFromLeft(boxBound.getWidth() * .5f);
        leftBound = leftBound.withSizeKeepingCentre(leftBound.getWidth() * .75f, leftBound.getHeight());
        ftypeC.setBounds(leftBound.toNearestInt());
        slopeC.setBounds(boxBound.toNearestInt());
        freqS.setBounds(bound.toNearestInt());
    }
} // zlPanel
