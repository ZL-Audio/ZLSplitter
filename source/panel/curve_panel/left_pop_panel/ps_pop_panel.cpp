// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ps_pop_panel.hpp"

namespace zlpanel {
    PSPopPanel::PSPopPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base), updater_(),
          balance_slider_("Balance", base),
          balance_attach_(balance_slider_.getSlider(), p.parameters_,
                          zlp::PPSBalance::kID, updater_),
          attack_slider_("Attack", base),
          attack_attach_(attack_slider_.getSlider(), p.parameters_,
                         zlp::PPSAttack::kID, updater_),
          hold_slider_("Hold", base),
          hold_attach_(hold_slider_.getSlider(), p.parameters_,
                       zlp::PPSHold::kID, updater_),
          smooth_slider_("Smooth", base),
          smooth_attach_(smooth_slider_.getSlider(), p.parameters_,
                         zlp::PPSSmooth::kID, updater_) {
        for (auto &s : {&balance_slider_, &attack_slider_, &hold_slider_, &smooth_slider_}) {
            addAndMakeVisible(*s);
        }

        setBufferedToImage(true);

        setInterceptsMouseClicks(false, true);
    }

    int PSPopPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return slider_height * 4 + padding * 3;
    }

    void PSPopPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        auto bound = getLocalBounds();
        balance_slider_.setBounds(bound.removeFromTop(slider_height));
        bound.removeFromTop(padding);
        attack_slider_.setBounds(bound.removeFromTop(slider_height));
        bound.removeFromTop(padding);
        hold_slider_.setBounds(bound.removeFromTop(slider_height));
        bound.removeFromTop(padding);
        smooth_slider_.setBounds(bound.removeFromTop(slider_height));\
    }

    void PSPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
