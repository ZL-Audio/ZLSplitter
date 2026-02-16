// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "lr_pop_panel.hpp"

namespace zlpanel {
    LRPopPanel::LRPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                           multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        mix_slider_("Mix", base,
                    tooltip_helper.getToolTipText(multilingual::kMix)),
        mix_attach_(mix_slider_.getSlider1(), p.parameters_,
                    zlp::PMix::kID, updater_) {
        addAndMakeVisible(mix_slider_);

        setInterceptsMouseClicks(false, true);

        setBufferedToImage(true);
    }

    int LRPopPanel::getIdealHeight() const {
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        return slider_width;
    }

    void LRPopPanel::resized() {
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        mix_slider_.setBounds(getLocalBounds().withSizeKeepingCentre(slider_width, slider_width));
    }

    void LRPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
} // zlpanel
