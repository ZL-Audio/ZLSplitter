// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ps_pop_panel.hpp"

namespace zlpanel {
    PSPopPanel::PSPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                           const multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        balance_slider_("", base,
                        tooltip_helper.getToolTipText(multilingual::kTSBalance), 1.25f),
        balance_attach_(balance_slider_.getSlider1(), p.parameters_,
                        zlp::PPSBalance::kID, updater_),
        attack_slider_("", base,
                         tooltip_helper.getToolTipText(multilingual::kTSStrength)),
        attack_attach_(attack_slider_.getSlider(), p.parameters_,
                         zlp::PPSAttack::kID, updater_),
        hold_slider_("", base,
                     tooltip_helper.getToolTipText(multilingual::kTSHold)),
        hold_attach_(hold_slider_.getSlider(), p.parameters_,
                     zlp::PPSHold::kID, updater_),
        smooth_slider_("", base,
                       tooltip_helper.getToolTipText(multilingual::kTSSmooth)),
        smooth_attach_(smooth_slider_.getSlider(), p.parameters_,
                       zlp::PPSSmooth::kID, updater_),
        label_laf_(base),
        balance_label_("", "Balance"),
        attack_label_("", "Attack"),
        hold_label_("", "Hold"),
        smooth_label_("", "Smooth") {
        addAndMakeVisible(balance_slider_);
        addAndMakeVisible(attack_slider_);
        addAndMakeVisible(hold_slider_);
        addAndMakeVisible(smooth_slider_);

        label_laf_.setFontScale(1.5f);
        balance_label_.setLookAndFeel(&label_laf_);
        balance_label_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(balance_label_);
        for (auto& l : {&attack_label_, &hold_label_, &smooth_label_}) {
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centredRight);
            addAndMakeVisible(l);
        }

        setInterceptsMouseClicks(false, true);
    }

    int PSPopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        return 4 * padding + slider_width + 4 * button_size;
    }

    void PSPopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);

        auto bound = getLocalBounds();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - padding, bound.getHeight() - padding);

        balance_label_.setBounds(bound.removeFromTop(button_size));
        balance_slider_.setBounds(bound.removeFromTop(slider_width));
        bound.removeFromTop(padding);
        const auto label_width = bound.getWidth() / 2 + padding / 2;
        {
            auto temp_bound = bound.removeFromTop(button_size);
            attack_label_.setBounds(temp_bound.removeFromLeft(label_width));
            attack_slider_.setBounds(temp_bound);
        }
        bound.removeFromTop(padding);
        {
            auto temp_bound = bound.removeFromTop(button_size);
            hold_label_.setBounds(temp_bound.removeFromLeft(label_width));
            hold_slider_.setBounds(temp_bound);
        }
        bound.removeFromTop(padding);
        {
            auto temp_bound = bound.removeFromTop(button_size);
            smooth_label_.setBounds(temp_bound.removeFromLeft(label_width));
            smooth_slider_.setBounds(temp_bound);
        }
    }

    void PSPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
