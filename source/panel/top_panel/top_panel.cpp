// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "BinaryData.h"
#include "top_panel.hpp"

namespace zlpanel {
    TopPanel::TopPanel(PluginProcessor& p, zlgui::UIState& base,
                       multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        logo_panel_(p, base, tooltip_helper),
        top_legend_panel_(p, base),
        top_choice_panel_(p, base, tooltip_helper),
        split_type_box_([]() -> std::vector<std::unique_ptr<juce::Drawable>> {
            std::vector<std::unique_ptr<juce::Drawable>> icons;
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::circle_svg, BinaryData::circle_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::leftright_svg, BinaryData::leftright_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::midside_svg, BinaryData::midside_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::lowhigh_svg, BinaryData::lowhigh_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::transteady_svg, BinaryData::transteady_svgSize));
            icons.emplace_back(
                juce::Drawable::createFromImageData(BinaryData::peaksteady_svg, BinaryData::peaksteady_svgSize));
            return icons;
        }(), base.gui),
        swap_button_(base.gui,
                     juce::Drawable::createFromImageData(BinaryData::shuffle_svg, BinaryData::shuffle_svgSize), true,
                     tooltip_helper.getToolTipText(multilingual::kSwap)),
        swap_attach_(swap_button_.getButton(), p.parameters_,
                     zlp::PSwap::kID, updater_),
        bypass_button_(base.gui,
                       juce::Drawable::createFromImageData(BinaryData::bypass_svg, BinaryData::bypass_svgSize), true,
                       tooltip_helper.getToolTipText(multilingual::kBypass)),
        bypass_attach_(bypass_button_.getButton(), p.parameters_,
                       zlp::PBypass::kID, updater_),
        analyzer_setting_button_(base.gui,
                                 juce::Drawable::createFromImageData(BinaryData::settings_svg, BinaryData::settings_svgSize),
                                 true),
        preset_button_(base.gui, juce::Drawable::createFromImageData(BinaryData::collections_bookmark_svg,
                                                               BinaryData::collections_bookmark_svgSize),
                       false, tooltip_helper.getToolTipText(multilingual::kPresetBrowser)) {
        lookAndFeelChanged();
        swap_button_.setComponentID(zlp::PSwap::kID);
        swap_button_.getButton().setComponentID(zlp::PSwap::kID);
        bypass_button_.setComponentID(zlp::PBypass::kID);
        bypass_button_.getButton().setComponentID(zlp::PBypass::kID);
        logo_panel_.setTooltip(tooltip_helper.getToolTipText(multilingual::kLogo));
        addAndMakeVisible(logo_panel_);
        addAndMakeVisible(top_legend_panel_);
        addAndMakeVisible(top_choice_panel_);

        split_type_box_.setBufferedToImage(true);
        split_type_box_.setScrollEnabled(true);
        split_type_box_.setComponentID(zlp::PSplitType::kID);
        split_type_box_.getBox().setComponentID(zlp::PSplitType::kID);
        split_type_box_.getBox().onChange = [this]() {
            const auto index = split_type_box_.getBox().getSelectedItemIndex();
            if (index < 0) { return; }
            const auto split_type = index == 0 ? zlp::PSplitType::kNone : index - 1;
            auto* parameter = p_ref_.parameters_.getParameter(zlp::PSplitType::kID);
            const auto value = parameter->convertTo0to1(static_cast<float>(split_type));
            if (std::abs(value - parameter->getValue()) <= 1e-6f) { return; }
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(value);
            parameter->endChangeGesture();
        };
        addAndMakeVisible(split_type_box_);

        swap_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        bypass_button_.setImageAlpha(1.f, 1.f, .5f, .75f);
        analyzer_setting_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        preset_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);

        for (auto& b : {&swap_button_, &bypass_button_, &analyzer_setting_button_, &preset_button_}) {
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        analyzer_setting_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel,
                                   static_cast<float>(analyzer_setting_button_.getToggleState()));
        };

        preset_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::kPresetBrowser, !static_cast<bool>(base_.getPanelProperty(zlgui::kPresetBrowser)));
        };

        setInterceptsMouseClicks(false, true);
        repaintCallBackSlow();
    }

    void TopPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.gui.getBackgroundColour());
    }

    int TopPanel::getIdealHeight() const {
        const auto font_size = base_.gui.getFontSize();
        return 2 * (getPaddingSize(font_size) / 2) + getButtonSize(font_size);
    }

    void TopPanel::resized() {
        const auto font_size = base_.gui.getFontSize();
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        bound.reduce(padding / 2, padding / 2);
        const auto button_size = bound.getHeight();
        const auto icon_inset = juce::roundToInt(font_size * .15f);

        logo_panel_.setBounds(bound.removeFromLeft(button_size * 2 + padding));
        bound.removeFromLeft(padding);
        preset_button_.setBounds(bound.removeFromLeft(button_size));
        preset_button_.getButton().setEdgeIndent(icon_inset);

        const auto control_width = button_size * 5 + padding * 2 + button_size / 2 + padding / 2;
        bound.setLeft(control_width - button_size - padding);
        split_type_box_.setBounds(bound.removeFromLeft(button_size));
        split_type_box_.getLAF().setIconSize(static_cast<float>(std::max(0, button_size - 2 * icon_inset)));
        bound.removeFromLeft(padding);
        swap_button_.setBounds(bound.removeFromLeft(bound.getHeight()));
        bound.removeFromLeft(padding);
        top_legend_panel_.setBounds(bound.removeFromLeft(top_legend_panel_.getIdealWidth()));
        bypass_button_.setBounds(bound.removeFromRight(bound.getHeight()));
        bound.removeFromRight(padding);
        analyzer_setting_button_.setBounds(bound.removeFromRight(bound.getHeight()));
        bound.removeFromRight(padding);
        top_choice_panel_.setBounds(bound.removeFromRight(top_choice_panel_.getIdealWidth()));
    }

    void TopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        const auto split_type = static_cast<int>(std::round(split_type_ref_.load(std::memory_order::relaxed)));
        split_type_box_.getBox().setSelectedItemIndex((split_type + 1) % 6, juce::dontSendNotification);

        top_legend_panel_.repaintCallBackSlow();
        top_choice_panel_.repaintCallBackSlow();
    }

    void TopPanel::lookAndFeelChanged() {
        const auto alignment = combobox_helper::getAlignment(base_.gui.getComboboxAlignment());
        split_type_box_.getLAF().setItemAlignment(alignment);
    }

}
