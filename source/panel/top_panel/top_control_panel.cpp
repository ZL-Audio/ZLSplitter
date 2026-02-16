// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "top_control_panel.hpp"

namespace zlpanel {
    TopControlPanel::TopControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                                     multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        swap_drawable_(juce::Drawable::createFromImageData(BinaryData::shuffle_svg, BinaryData::shuffle_svgSize)),
        swap_button_("", base,
                     tooltip_helper.getToolTipText(multilingual::kSwap)),
        swap_attach_(swap_button_.getButton(), p.parameters_,
                     zlp::PSwap::kID, updater_),
        bypass_drawable_(
            juce::Drawable::createFromImageData(BinaryData::mode_off_on_svg, BinaryData::mode_off_on_svgSize)),
        bypass_button_("", base,
                       tooltip_helper.getToolTipText(multilingual::kBypass)),
        bypass_attach_(bypass_button_.getButton(), p.parameters_,
                       zlp::PBypass::kID, updater_) {
        swap_button_.setDrawable(swap_drawable_.get());
        bypass_button_.setDrawable(bypass_drawable_.get());
        bypass_button_.getLAF().setReverse(true);

        for (auto& b : {&swap_button_, &bypass_button_}) {
            b->getLAF().setScale(1.5f);
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        setInterceptsMouseClicks(false, true);
    }

    int TopControlPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto button_width = juce::roundToInt(base_.getFontSize() * kButtonScale);
        return button_width * 2 + padding * 2;
    }

    void TopControlPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto button_width = juce::roundToInt(base_.getFontSize() * kButtonScale);

        auto bound = getLocalBounds();
        bound.removeFromRight(padding);
        bypass_button_.setBounds(bound.removeFromRight(button_width));
        bound.removeFromRight(padding);
        swap_button_.setBounds(bound.removeFromRight(button_width));
    }

    void TopControlPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
