// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "control_pop_panel.hpp"

namespace zlpanel {
    ControlPopPanel::ControlPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                                     const multilingual::TooltipHelper& tooltip_helper) :
        base_{base},
        background_(base),
        lr_pop_panel_(p, base, tooltip_helper),
        lh_pop_panel_(p, base, tooltip_helper),
        ts_pop_panel_(p, base, tooltip_helper),
        ps_pop_panel_(p, base, tooltip_helper) {
        background_.setBufferedToImage(true);
        addAndMakeVisible(background_);
        addChildComponent(lr_pop_panel_);
        addChildComponent(lh_pop_panel_);
        addChildComponent(ts_pop_panel_);
        addChildComponent(ps_pop_panel_);
    }

    int ControlPopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        auto height = 2 * padding;
        switch (split_type_) {
        case zlp::PSplitType::SplitType::kNone: {
            height = 0;
            break;
        }
        case zlp::PSplitType::SplitType::kLRight:
        case zlp::PSplitType::SplitType::kMSide: {
            height += lr_pop_panel_.getIdealHeight();
            break;
        }
        case zlp::PSplitType::SplitType::kLHigh: {
            height += lh_pop_panel_.getIdealHeight();
            break;
        }
        case zlp::PSplitType::SplitType::kTSteady: {
            height += ts_pop_panel_.getIdealHeight();
            break;
        }
        case zlp::PSplitType::SplitType::kPSteady: {
            height += ps_pop_panel_.getIdealHeight();
            break;
        }
        }
        return height;
    }

    void ControlPopPanel::resized() {
        auto bound = getLocalBounds();
        background_.setBounds(bound);

        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        bound.reduce(padding, padding);
        switch (split_type_) {
        case zlp::PSplitType::SplitType::kNone: {
            break;
        }
        case zlp::PSplitType::SplitType::kLRight:
        case zlp::PSplitType::SplitType::kMSide: {
            lr_pop_panel_.setBounds(bound);
            break;
        }
        case zlp::PSplitType::SplitType::kLHigh: {
            lh_pop_panel_.setBounds(bound);
            break;
        }
        case zlp::PSplitType::SplitType::kTSteady: {
            ts_pop_panel_.setBounds(bound);
            break;
        }
        case zlp::PSplitType::SplitType::kPSteady: {
            ps_pop_panel_.setBounds(bound);
            break;
        }
        }
    }

    void ControlPopPanel::repaintCallBackSlow() {
        switch (split_type_) {
        case zlp::PSplitType::SplitType::kNone: {
            break;
        }
        case zlp::PSplitType::SplitType::kLRight:
        case zlp::PSplitType::SplitType::kMSide: {
            lr_pop_panel_.repaintCallBackSlow();
            break;
        }
        case zlp::PSplitType::SplitType::kLHigh: {
            lh_pop_panel_.repaintCallBackSlow();
            break;
        }
        case zlp::PSplitType::SplitType::kTSteady: {
            ts_pop_panel_.repaintCallBackSlow();
            break;
        }
        case zlp::PSplitType::SplitType::kPSteady: {
            ps_pop_panel_.repaintCallBackSlow();
            break;
        }
        }
        if (isMouseOver(true)) {
            setAlpha(1.f);
        } else {
            setAlpha(.5f);
        }
    }

    void ControlPopPanel::setSplitType(const zlp::PSplitType::SplitType split_type) {
        split_type_ = split_type;
        lr_pop_panel_.setVisible(split_type == zlp::PSplitType::SplitType::kLRight ||
            split_type_ == zlp::PSplitType::SplitType::kMSide);
        lh_pop_panel_.setVisible(split_type == zlp::PSplitType::SplitType::kLHigh);
        ts_pop_panel_.setVisible(split_type == zlp::PSplitType::SplitType::kTSteady);
        ps_pop_panel_.setVisible(split_type == zlp::PSplitType::SplitType::kPSteady);
    }
}
