// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "top_legend_panel.hpp"

namespace zlpanel {
    TopLegendPanel::TopLegendPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base),
          split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
          swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)) {
        setBufferedToImage(true);

        setInterceptsMouseClicks(false, false);
    }

    void TopLegendPanel::paint(juce::Graphics &g) {
        const auto idx = static_cast<size_t>(std::round(c_split_type_));
        g.setFont(base_.getFontSize() * 1.5f);

        std::vector<juce::String> labels;
        std::vector<juce::Colour> colours;

        if (idx >= 5) {
            labels.emplace_back("Input");
            colours.emplace_back(base_.getTextColor());
        } else {
            labels.emplace_back(c_swap_ ? kText2[idx] : kText1[idx]);
            colours.emplace_back(base_.getColourByIdx(zlgui::ColourIdx::kOutput1Colour));
            labels.emplace_back(c_swap_ ? kText1[idx] : kText2[idx]);
            colours.emplace_back(base_.getColourByIdx(zlgui::ColourIdx::kOutput2Colour));
        }

        const auto padding = std::round(base_.getFontSize() * kPaddingScale);
        const auto slider_width = std::round(base_.getFontSize() * kSliderScale);
        const auto button_width = std::round(base_.getFontSize() * kButtonScale);

        auto bound = getLocalBounds().toFloat();
        const auto legend_size = button_width * .5f;
        for (size_t i = 0; i < labels.size(); ++i) {
            bound.removeFromLeft(padding);
            g.setColour(base_.getTextColor());
            g.drawText(labels[i], bound.removeFromLeft(slider_width), juce::Justification::centredRight);
            bound.removeFromLeft(padding);
            auto legend_bound = bound.removeFromLeft(button_width);
            legend_bound = legend_bound.withSizeKeepingCentre(legend_size, legend_size);
            g.setColour(colours[i]);
            g.fillRoundedRectangle(legend_bound, legend_size * .2f);
            bound.removeFromLeft(padding);
        }
    }

    int TopLegendPanel::getIdealWidth() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto button_width = juce::roundToInt(base_.getFontSize() * kButtonScale);
        return padding * 6 + slider_width * 2 + button_width * 2;
    }

    void TopLegendPanel::repaintCallBackSlow() {
        const auto new_split_type = split_type_ref_.load(std::memory_order_relaxed);
        const auto new_swap = swap_ref_.load(std::memory_order_relaxed) > .5f;
        if (std::abs(new_split_type - c_split_type_) > .1f || new_swap != c_swap_) {
            c_split_type_ = new_split_type;
            c_swap_ = new_swap;
            repaint();
        }
    }
}
