// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ts_pop_panel.hpp"

namespace zlpanel {
    TSPopPanel::TSPopPanel(PluginProcessor &p, zlgui::UIBase &base,
                           multilingual::TooltipHelper &tooltip_helper)
        : base_(base), updater_(),
          balance_slider_("Balance", base,
                          tooltip_helper.getToolTipText(multilingual::kTSBalance)),
          balance_attach_(balance_slider_.getSlider(), p.parameters_,
                          zlp::PTSBalance::kID, updater_),
          strength_slider_("Strength", base,
                           tooltip_helper.getToolTipText(multilingual::kTSStrength)),
          strength_attach_(strength_slider_.getSlider(), p.parameters_,
                           zlp::PTSStrength::kID, updater_),
          hold_slider_("Hold", base,
                       tooltip_helper.getToolTipText(multilingual::kTSHold)),
          hold_attach_(hold_slider_.getSlider(), p.parameters_,
                       zlp::PTSHold::kID, updater_),
          smooth_slider_("Smooth", base,
                         tooltip_helper.getToolTipText(multilingual::kTSSmooth)),
          smooth_attach_(smooth_slider_.getSlider(), p.parameters_,
                         zlp::PTSSmooth::kID, updater_) {
        for (auto &s: {&balance_slider_, &strength_slider_, &hold_slider_, &smooth_slider_}) {
            addAndMakeVisible(*s);
        }

        setBufferedToImage(true);

        setInterceptsMouseClicks(false, true);
    }

    int TSPopPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return slider_height * 4 + padding * 3;
    }

    void TSPopPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        auto bound = getLocalBounds();
        balance_slider_.setBounds(bound.removeFromTop(slider_height));
        bound.removeFromTop(padding);
        strength_slider_.setBounds(bound.removeFromTop(slider_height));
        bound.removeFromTop(padding);
        hold_slider_.setBounds(bound.removeFromTop(slider_height));
        bound.removeFromTop(padding);
        smooth_slider_.setBounds(bound.removeFromTop(slider_height));\
    }

    void TSPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
