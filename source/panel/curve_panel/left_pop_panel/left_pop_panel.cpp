// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "left_pop_panel.hpp"

namespace zlpanel {
    LeftPopPanel::LeftPopPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base),
          split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
          lr_pop_panel_(p, base),
          lh_pop_panel_(p, base),
          ts_pop_panel_(p, base),
          ps_pop_panel_(p, base) {
        addChildComponent(lr_pop_panel_);
        addChildComponent(lh_pop_panel_);
        addChildComponent(ts_pop_panel_);
        addChildComponent(ps_pop_panel_);

        setInterceptsMouseClicks(false, true);
    }

    void LeftPopPanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillPath(background_);
    }

    void LeftPopPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        auto bound = getLocalBounds();

        bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * padding, bound.getHeight()); {
            const auto height = lr_pop_panel_.getIdealHeight();
            lr_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        } {
            const auto height = lh_pop_panel_.getIdealHeight();
            lh_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        } {
            const auto height = ts_pop_panel_.getIdealHeight();
            ts_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        } {
            const auto height = ps_pop_panel_.getIdealHeight();
            ps_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        }

        const auto float_bound = getLocalBounds().toFloat();
        background_.clear();
        background_.startNewSubPath(float_bound.getTopLeft());
        background_.lineTo({float_bound.getRight(), float_bound.getY() + float_bound.getWidth()});
        background_.lineTo({float_bound.getRight(), float_bound.getBottom() - float_bound.getWidth()});
        background_.lineTo(float_bound.getBottomLeft());
        background_.closeSubPath();
    }

    void LeftPopPanel::repaintCallBackSlow() {
        const auto new_split_type = split_type_ref_.load(std::memory_order::relaxed);
        if (std::abs(new_split_type - c_split_type_) > .1f) {
            c_split_type_ = new_split_type;
            const auto split_type = static_cast<zlp::PSplitType::SplitType>(std::round(c_split_type_));
            lr_pop_panel_.setVisible(split_type == zlp::PSplitType::kLRight || split_type == zlp::PSplitType::kMSide);
            lh_pop_panel_.setVisible(split_type == zlp::PSplitType::kLHigh);
            ts_pop_panel_.setVisible(split_type == zlp::PSplitType::kTSteady);
            ps_pop_panel_.setVisible(split_type == zlp::PSplitType::kPSteady);
        }
        if (isVisible()) {
            lr_pop_panel_.repaintCallBackSlow();
            lh_pop_panel_.repaintCallBackSlow();
            ts_pop_panel_.repaintCallBackSlow();
            ps_pop_panel_.repaintCallBackSlow();
        }
    }
} // zlpanel
