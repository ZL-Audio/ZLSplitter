// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

namespace zlpanel {
    ControlPanel::ControlPanel(PluginProcessor& p, zlgui::UIBase& base, multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        split_type_idx_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        control_pop_panel_(p, base, tooltip_helper) {
        control_pop_panel_.setBufferedToImage(true);
        addAndMakeVisible(control_pop_panel_);

        setInterceptsMouseClicks(false, true);
    }

    int ControlPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);

        return padding / 2 + button_size * 2 + padding + button_size + button_size + button_size / 2 + padding + button_size;
    }

    void ControlPanel::resized() {
        const auto ideal_height = control_pop_panel_.getIdealHeight();
        const auto bound = juce::Rectangle{0, 0, getWidth(), ideal_height};
        if (bound != control_pop_panel_.getBounds()) {
            control_pop_panel_.setBounds(bound);
        } else {
            control_pop_panel_.resized();
        }
    }

    void ControlPanel::repaintCallBackSlow() {
        const auto split_type = static_cast<zlp::PSplitType::SplitType>(std::round(
            split_type_idx_.load(std::memory_order::relaxed)));
        if (split_type != split_type_) {
            split_type_ = split_type;
            control_pop_panel_.setSplitType(split_type_);
            resized();
        }
        control_pop_panel_.repaintCallBackSlow();
    }
}
