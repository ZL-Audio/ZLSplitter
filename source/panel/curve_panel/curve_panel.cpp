// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor &p, zlgui::UIBase &base,
                           multilingual::TooltipHelper &tooltip_helper)
        : Thread("curve_panel"), p_ref_(p), base_(base),
          split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
          analyzer_show_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerShow::kID)),
          fft_panel_(p, base),
          mag_panel_(p, base),
          left_control_panel_(p, base, tooltip_helper),
          left_pop_panel_(p, base, tooltip_helper) {
        juce::ignoreUnused(p_ref_, tooltip_helper);
        addChildComponent(fft_panel_);
        addChildComponent(mag_panel_);
        fft_panel_.addMouseListener(this, false);
        left_control_panel_.setAlpha(0.f);
        addAndMakeVisible(left_control_panel_);
        addChildComponent(left_pop_panel_);

        setOpaque(true);

        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    void CurvePanel::paint(juce::Graphics &g) {
        g.setColour(base_.getBackgroundColor());
        g.fillRect(getLocalBounds());
    }

    void CurvePanel::paintOverChildren(juce::Graphics &g) {
        juce::ignoreUnused(g);
        notify();
    }

    void CurvePanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto button_width = juce::roundToInt(base_.getFontSize() * kButtonScale);
        auto bound = getLocalBounds();
        fft_panel_.setBounds(bound);
        mag_panel_.setBounds(bound);
        bound.removeFromBottom(button_width);
        left_control_panel_.setBounds(bound.withRight(bound.getX() + button_width));
        bound.removeFromLeft(left_control_panel_.getWidth());
        left_pop_panel_.setBounds(bound.removeFromLeft(slider_width + 2 * padding));
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            const auto c_analyzer_show = analyzer_show_ref_.load(std::memory_order::relaxed);
            if (c_analyzer_show < .5f) {
                fft_panel_.run();
            } else {
                mag_panel_.run(next_time_stamp_.load(std::memory_order::relaxed));
            }
            if (threadShouldExit()) {
                return;
            }
        }
    }

    void CurvePanel::mouseDown(const juce::MouseEvent &) {
        left_control_panel_.setAlpha(0.f);
        left_pop_panel_.setVisible(false);
    }

    void CurvePanel::repaintCallBackSlow() {
        const auto c_analyzer_show = analyzer_show_ref_.load(std::memory_order::relaxed);
        if (c_analyzer_show < .5f) {
            fft_panel_.setVisible(true);
            mag_panel_.setVisible(false);
        } else {
            fft_panel_.setVisible(false);
            mag_panel_.setVisible(true);
        }
        fft_panel_.repaintCallBackSlow();
        mag_panel_.repaintCallBackSlow();
        if (left_control_panel_.isMouseOver(true)) {
            left_control_panel_.setAlpha(1.f);
            left_pop_panel_.setVisible(static_cast<zlp::PSplitType::SplitType>(
                                           std::round(split_type_ref_.load(std::memory_order_relaxed))) !=
                                       zlp::PSplitType::kNone);
        }
        left_control_panel_.repaintCallBackSlow();
        left_pop_panel_.repaintCallBackSlow();
    }

    void CurvePanel::repaintCallBack(const double time_stamp) {
        next_time_stamp_.store(time_stamp, std::memory_order::relaxed);
        if (isVisible()) {
            if (fft_panel_.isVisible()) {
                fft_panel_.repaint();
            }
            if (mag_panel_.isVisible()) {
                mag_panel_.repaint();
            }
        }
    }
} // zlpanel
