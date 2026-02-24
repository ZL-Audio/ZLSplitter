// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "lh_pop_panel.hpp"

namespace zlpanel {
    LHPopPanel::LHPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                           const multilingual::TooltipHelper& tooltip_helper) :
        base_(base), updater_(),
        mix_slider_("", base,
                    tooltip_helper.getToolTipText(multilingual::kMix), 1.25f),
        mix_attach_(mix_slider_.getSlider1(), p.parameters_,
                    zlp::PMix::kID, updater_),
        filter_type_box_(zlp::PLHFilterType::kChoices, base,
                         tooltip_helper.getToolTipText(multilingual::kLHFilterType)),
        filter_type_attach_(filter_type_box_.getBox(), p.parameters_,
                            zlp::PLHFilterType::kID, updater_),
        filter_slope_box_(zlp::PLHSlope::kChoices, base,
                          tooltip_helper.getToolTipText(multilingual::kLHFilterSlope)),
        filter_slope_attach_(filter_slope_box_.getBox(), p.parameters_,
                             zlp::PLHSlope::kID, updater_),
        freq_slider_("", base,
                     tooltip_helper.getToolTipText(multilingual::kLHFreq), 1.25f),
        freq_attach_(freq_slider_.getSlider1(), p.parameters_,
                     zlp::PLHFreq::kID, updater_),
        label_laf_(base),
        mix_label_("", "Mix"),
        freq_label_("", "Freq") {
        addAndMakeVisible(mix_slider_);
        addAndMakeVisible(filter_type_box_);
        addAndMakeVisible(filter_slope_box_);
        addAndMakeVisible(freq_slider_);

        label_laf_.setFontScale(1.5f);
        mix_label_.setLookAndFeel(&label_laf_);
        mix_label_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(mix_label_);
        freq_label_.setLookAndFeel(&label_laf_);
        freq_label_.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(freq_label_);

        setInterceptsMouseClicks(false, true);
    }

    int LHPopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        return 3 * padding + 2 * slider_width + 3 * button_size;
    }

    void LHPopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);
        const auto button_size = getButtonSize(font_size);
        auto bound = getLocalBounds();
        bound = bound.withSizeKeepingCentre(slider_width, bound.getHeight() - padding);

        mix_label_.setBounds(bound.removeFromTop(button_size));
        mix_slider_.setBounds(bound.removeFromTop(slider_width));
        bound.removeFromTop(padding);
        {
            auto temp_bound = bound.removeFromTop(button_size);
            const auto box_width = (temp_bound.getWidth() - padding) / 2;
            filter_type_box_.setBounds(temp_bound.removeFromLeft(box_width));
            filter_slope_box_.setBounds(temp_bound.removeFromRight(box_width));
        }
        bound.removeFromTop(padding);
        freq_label_.setBounds(bound.removeFromTop(button_size));
        freq_slider_.setBounds(bound.removeFromTop(slider_width));
    }

    void LHPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
