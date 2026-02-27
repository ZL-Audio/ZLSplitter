// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "mag_background_panel.hpp"

namespace zlpanel {
    MagBackgroundPanel::MagBackgroundPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PMagMinDB::kID)) {
        setInterceptsMouseClicks(false, false);
        lookAndFeelChanged();
    }

    void MagBackgroundPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
        drawDBs(g);
    }

    void MagBackgroundPanel::repaintCallBackSlow() {
        const auto min_db = static_cast<int>(std::round(min_db_ref_.load(std::memory_order::relaxed)));
        if (c_min_db_ != min_db) {
            c_min_db_ = min_db;
            repaint();
        }
    }

    void MagBackgroundPanel::drawDBs(juce::Graphics& g) const {
        const auto bound = getLocalBounds().toFloat();
        // draw db grid
        const auto thickness = base_.getFontSize() * 0.1f;
        const auto unit_height = bound.getHeight() / 7.f;
        juce::RectangleList<float> rect_list;
        for (size_t i = 1; i < 7; ++i) {
            const auto y = unit_height * static_cast<float>(i);
            rect_list.add(0.f, y - thickness * .5f, bound.getWidth(), thickness);
        }
        g.setColour(grid_colour_);
        g.fillRectList(rect_list);
        // draw right gradient
        const auto shadow_rect = bound.withLeft(bound.getRight() - base_.getFontSize() * 2.f);
        juce::ColourGradient gradient;
        gradient.point1 = juce::Point<float>(shadow_rect.getX(), shadow_rect.getY());
        gradient.point2 = juce::Point<float>(shadow_rect.getRight(), shadow_rect.getY());
        gradient.isRadial = false;
        gradient.clearColours();
        gradient.addColour(0.0, base_.getBackgroundColour().withAlpha(0.f));
        gradient.addColour(1.0, base_.getBackgroundColour().withAlpha(1.f));
        g.setGradientFill(gradient);
        g.fillRect(shadow_rect);
        // draw db values
        g.setColour(base_.getTextColour().withAlpha(.5f));
        g.setFont(base_.getFontSize() * 1.25f);
        const auto label_x0 = bound.getRight() - base_.getFontSize() * 3.25f;
        const auto label_height = base_.getFontSize() * 1.1f;
        const auto label_width = base_.getFontSize() * 3.f;
        const auto min_db_unit = zlstate::PMagMinDB::kDBs[static_cast<size_t>(c_min_db_)] / 6.f;
        for (size_t i = 1; i < 7; ++i) {
            const auto db = min_db_unit * static_cast<float>(i);
            const auto y = unit_height * static_cast<float>(i);
            const auto rect = juce::Rectangle(label_x0, y - label_height * .5f,
                                              label_width, label_height);
            g.drawText(juce::String(juce::roundToInt(db)),
                       rect, juce::Justification::centredRight, false);
        }
    }

    void MagBackgroundPanel::lookAndFeelChanged() {
        const auto grid_colour = base_.getColourByIdx(zlgui::ColourIdx::kGridColour);
        const auto background_colour = base_.getBackgroundColour();
        const auto alpha = grid_colour.getFloatAlpha();
        grid_colour_ = juce::Colour::fromFloatRGBA(
            grid_colour.getFloatRed() * alpha + background_colour.getFloatRed() * (1.f - alpha),
            grid_colour.getFloatGreen() * alpha + background_colour.getFloatGreen() * (1.f - alpha),
            grid_colour.getFloatBlue() * alpha + background_colour.getFloatBlue() * (1.f - alpha),
            1.f);
    }
}
