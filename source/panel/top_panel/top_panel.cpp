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
    TopPanel::TopPanel(PluginProcessor& p, zlgui::UIBase& base,
                       multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
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
        }(), base),
        split_type_attachment_(split_type_box_.getBox(), p.parameters_, zlp::PSplitType::kID, updater_,
                               {1, 2, 3, 4, 5, 0}, {5, 0, 1, 2, 3, 4}),
        swap_drawable_(juce::Drawable::createFromImageData(BinaryData::shuffle_svg, BinaryData::shuffle_svgSize)),
        swap_button_(base, swap_drawable_.get(), swap_drawable_.get(),
                     tooltip_helper.getToolTipText(multilingual::kSwap)),
        swap_attach_(swap_button_.getButton(), p.parameters_,
                     zlp::PSwap::kID, updater_),
        bypass_drawable_(
            juce::Drawable::createFromImageData(BinaryData::bypass_svg, BinaryData::bypass_svgSize)),
        bypass_button_(base, bypass_drawable_.get(), bypass_drawable_.get(),
                       tooltip_helper.getToolTipText(multilingual::kBypass)),
        bypass_attach_(bypass_button_.getButton(), p.parameters_,
                       zlp::PBypass::kID, updater_),
        analyzer_setting_drawable_(
            juce::Drawable::createFromImageData(BinaryData::settings_svg, BinaryData::settings_svgSize)),
        analyzer_setting_button_(base, analyzer_setting_drawable_.get(), analyzer_setting_drawable_.get()) {
        logo_panel_.setTooltip(tooltip_helper.getToolTipText(multilingual::kLogo));
        addAndMakeVisible(logo_panel_);
        addAndMakeVisible(top_legend_panel_);
        addAndMakeVisible(top_choice_panel_);

        split_type_box_.setBufferedToImage(true);
        addAndMakeVisible(split_type_box_);

        swap_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);
        bypass_button_.setImageAlpha(1.f, 1.f, .5f, .75f);
        analyzer_setting_button_.setImageAlpha(.5f, .75f, 1.f, 1.f);

        for (auto& b : {&swap_button_, &bypass_button_, &analyzer_setting_button_}) {
            b->setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        analyzer_setting_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel,
                                   static_cast<float>(analyzer_setting_button_.getToggleState()));
        };

        setInterceptsMouseClicks(false, true);
    }

    void TopPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
    }

    int TopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        return 2 * (getPaddingSize(font_size) / 2) + getButtonSize(font_size);
    }

    void TopPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);

        auto bound = getLocalBounds();
        bound.reduce(padding / 2, padding / 2);

        logo_panel_.setBounds(bound.removeFromLeft(bound.getHeight() * 2 + padding));
        bound.removeFromLeft(bound.getHeight());
        split_type_box_.setBounds(bound.removeFromLeft(bound.getHeight() + bound.getHeight() / 2));
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

        top_legend_panel_.repaintCallBackSlow();
        top_choice_panel_.repaintCallBackSlow();
    }
}
