// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "mag_background_panel.hpp"

namespace zlpanel {
    MagBackgroundPanel::MagBackgroundPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base), updater_(),
          mag_time_length_box_(zlstate::PMagTimeLength::kChoices, base),
          mag_time_length_attach_(mag_time_length_box_.getBox(), p.na_parameters_,
                                  zlstate::PMagTimeLength::kID, updater_),
          min_db_box_(zlstate::PMagMinDB::kChoices, base),
          min_db_attach_(min_db_box_.getBox(), p.na_parameters_,
                         zlstate::PMagMinDB::kID, updater_),
          mag_type_box_(zlstate::PMagType::kChoices, base),
          mag_type_attach_(mag_type_box_.getBox(), p.na_parameters_,
                           zlstate::PMagType::kID, updater_) {
        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);

        const auto box_alpha = std::clamp(
            base_.getColourByIdx(zlgui::ColourIdx::kGridColour).getFloatAlpha() + .2f, .2f, 1.f);

        mag_time_length_box_.getLAF().setLabelJustification(juce::Justification::bottomLeft);
        min_db_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);
        mag_type_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);

        for (auto &b: {&mag_time_length_box_, &min_db_box_, &mag_type_box_}) {
            b->setAlpha(box_alpha);
            b->getLAF().setOption(popup_option);
            b->getLAF().setFontScale(1.f);
            addAndMakeVisible(b);
        }

        setBufferedToImage(true);
        setInterceptsMouseClicks(false, true);
    }

    void MagBackgroundPanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
        if (c_mag_min_db_ < 0) {
            return;
        }
        const auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.1f;
        const auto min_db = zlstate::PMagMinDB::kDBs[static_cast<size_t>(c_mag_min_db_)];

        g.setFont(base_.getFontSize());

        const auto grid_colour = base_.getColourByIdx(zlgui::ColourIdx::kGridColour);
        const auto text_colour = base_.getTextColor().withAlpha(
            std::clamp(grid_colour.getFloatAlpha() + .2f, .2f, 1.f));

        const auto right_padding = std::round(base_.getFontSize() * .25f);
        for (size_t i = 1; i < 6; ++i) {
            const auto y = bound.getY() + static_cast<float>(i) / 6.f * bound.getHeight();
            g.setColour(grid_colour);
            g.fillRect(juce::Rectangle<float>{bound.getX(), y - thickness * .5f, bound.getWidth(), thickness});
            const auto text_bound = juce::Rectangle<float>(bound.getRight() - base_.getFontSize() * 3 - right_padding,
                                                           y - base_.getFontSize() * 2,
                                                           base_.getFontSize() * 3, base_.getFontSize() * 2);
            g.setColour(text_colour);
            g.drawText(juce::String(juce::roundToInt(static_cast<float>(i) / 6.f * min_db)),
                       text_bound, juce::Justification::bottomRight);
        }
    }

    void MagBackgroundPanel::resized() {
        const auto bound = getLocalBounds();
        const auto box_height = juce::roundToInt(base_.getFontSize() * 1.25f);
        const auto box_width = juce::roundToInt(base_.getFontSize() * 2.5f);
        const auto padding1 = box_width / 10;
        const auto padding2 = juce::roundToInt(std::ceil(base_.getFontSize()));
        const auto padding3 = static_cast<int>(std::round(base_.getFontSize() * .25f));
        mag_time_length_box_.setBounds({
            bound.getX() + padding1, bound.getBottom() - box_height,
            box_width, box_height
        });
        mag_type_box_.setBounds({
            bound.getRight() - box_width - padding2, bound.getBottom() - box_height,
            box_width, box_height
        });
        min_db_box_.setBounds({
            bound.getRight() - box_width - padding3, bound.getBottom() - box_height - padding2,
            box_width, box_height
        });
    }

    void MagBackgroundPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        if (c_mag_min_db_ != min_db_box_.getBox().getSelectedItemIndex()) {
            c_mag_min_db_ = min_db_box_.getBox().getSelectedItemIndex();
            repaint();
        }
    }
}
