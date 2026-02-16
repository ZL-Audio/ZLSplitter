// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "left_pop_panel.hpp"

namespace zlpanel {
    LeftPopPanel::LeftPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                               multilingual::TooltipHelper& tooltip_helper) :
        base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        lr_pop_panel_(p, base, tooltip_helper),
        lh_pop_panel_(p, base, tooltip_helper),
        ts_pop_panel_(p, base, tooltip_helper),
        ps_pop_panel_(p, base, tooltip_helper) {
        addChildComponent(lr_pop_panel_);
        addChildComponent(lh_pop_panel_);
        addChildComponent(ts_pop_panel_);
        addChildComponent(ps_pop_panel_);

        setBufferedToImage(true);
        setInterceptsMouseClicks(false, true);
    }

    void LeftPopPanel::paint(juce::Graphics& g) {
        g.setColour(base_.getBackgroundColor());
        g.fillPath(background_);
    }

    void LeftPopPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        auto bound = getLocalBounds();

        bound = bound.withSizeKeepingCentre(bound.getWidth() - 2 * padding, bound.getHeight());
        {
            const auto height = lr_pop_panel_.getIdealHeight();
            lr_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        }
        {
            const auto height = lh_pop_panel_.getIdealHeight();
            lh_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        }
        {
            const auto height = ts_pop_panel_.getIdealHeight();
            ts_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        }
        {
            const auto height = ps_pop_panel_.getIdealHeight();
            ps_pop_panel_.setBounds(bound.withSizeKeepingCentre(bound.getWidth(), height));
        }

        auto max_height = lr_pop_panel_.getBounds().getHeight();
        max_height = std::max(max_height, lh_pop_panel_.getBounds().getHeight());
        max_height = std::max(max_height, ts_pop_panel_.getBounds().getHeight());
        max_height = std::max(max_height, ps_pop_panel_.getBounds().getHeight());

        const auto float_bound = getLocalBounds().toFloat();
        const auto float_padding = std::round(base_.getFontSize() * kPaddingScale * 1.5f);

        std::array<float, 6> ys;
        ys[4] = (float_bound.getHeight() - static_cast<float>(max_height)) * .5f - float_padding;
        ys[1] = std::max(float_padding, ys[4] - float_bound.getWidth() * 0.618f);
        ys[0] = ys[1] - float_padding;
        ys[2] = ys[1] + float_padding / bound.getWidth() * (ys[4] - ys[1]);
        ys[3] = ys[4] - (ys[2] - ys[1]);
        ys[5] = ys[4] + float_padding;

        background_.clear();
        background_.startNewSubPath(0.f, ys[0] - float_padding);
        background_.quadraticTo(0.f, ys[1],
                                float_padding, ys[2]);
        background_.lineTo(float_bound.getRight() - float_padding, ys[3]);
        background_.quadraticTo(float_bound.getRight(),
                                ys[4], float_bound.getRight(), ys[5]);
        background_.lineTo(float_bound.getRight(), float_bound.getHeight() - ys[5]);
        background_.quadraticTo(float_bound.getRight(), float_bound.getHeight() - ys[4],
                                float_bound.getRight() - float_padding, float_bound.getHeight() - ys[3]);
        background_.lineTo(float_padding, float_bound.getHeight() - ys[2]);
        background_.quadraticTo(0.f, float_bound.getHeight() - ys[1],
                                0.f, float_bound.getHeight() - ys[0]);
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
