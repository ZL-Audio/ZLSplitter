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
    LRPopPanel::LRPopPanel(PluginProcessor& p, zlgui::UIBase& base, multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        mix_slider_("", base,
                    tooltip_helper.getToolTipText(multilingual::kMix), 1.25f),
        mix_attach_(mix_slider_.getSlider1(), p.parameters_,
                    zlp::PMix::kID, updater_),
        label_laf_(base),
        mix_label_("", "Mix") {
        mix_label_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(mix_label_);
        addAndMakeVisible(mix_slider_);

        setInterceptsMouseClicks(false, true);
    }

    int LRPopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        return 2 * padding + slider_width + button_size;
    }

    void LRPopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);

        auto bound = getLocalBounds();
        bound = bound.withSizeKeepingCentre(slider_width, bound.getHeight() - padding);

        mix_label_.setBounds(bound.removeFromTop(button_size));
        mix_slider_.setBounds(bound.removeFromBottom(slider_width));
    }

    void LRPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
