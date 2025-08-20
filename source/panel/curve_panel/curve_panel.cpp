// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "curve_panel.hpp"

namespace zlpanel {
    CurvePanel::CurvePanel(PluginProcessor &p, zlgui::UIBase &base,
                           multilingual::TooltipHelper &tooltip_helper)
        : Thread("curve_panel"), p_ref_(p), base_(base),
          fft_panel_(p, base),
          left_control_panel_(p, base),
          left_pop_panel_(p, base) {
        juce::ignoreUnused(p_ref_, tooltip_helper);
        addAndMakeVisible(fft_panel_);
        fft_panel_.addMouseListener(this, false);
        addAndMakeVisible(left_control_panel_);
        addChildComponent(left_pop_panel_);
        startThread(juce::Thread::Priority::low);
    }

    CurvePanel::~CurvePanel() {
        if (isThreadRunning()) {
            stopThread(-1);
        }
    }

    void CurvePanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
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
        left_control_panel_.setBounds(bound.removeFromLeft(button_width));
        fft_panel_.setBounds(bound);
        left_pop_panel_.setBounds(bound.removeFromLeft(slider_width + 2 * padding));
    }

    void CurvePanel::run() {
        juce::ScopedNoDenormals no_denormals;
        while (!threadShouldExit()) {
            const auto flag = wait(-1);
            juce::ignoreUnused(flag);
            fft_panel_.run();
            if (threadShouldExit()) {
                return;
            }
        }
    }

    void CurvePanel::mouseDown(const juce::MouseEvent &) {
        left_pop_panel_.setVisible(false);
    }

    void CurvePanel::repaintCallBackSlow() {
        fft_panel_.repaintCallBackSlow();
        left_control_panel_.repaintCallBackSlow();
        if (left_control_panel_.isMouseOver(true)) {
            left_pop_panel_.setVisible(true);
        }
        left_pop_panel_.repaintCallBackSlow();
    }

    void CurvePanel::repaintCallBack(const double time_stamp) {
        juce::ignoreUnused(time_stamp);
        repaint();
    }
} // zlpanel
