// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "fft_background_panel.hpp"

namespace zlpanel {
    FFTBackgroundPanel::FFTBackgroundPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        fft_min_db_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTMinDB::kID)) {
        setInterceptsMouseClicks(false, true);
        lookAndFeelChanged();
    }

    void FFTBackgroundPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColour());
        if (freq_max_ < 10000.0 || c_fft_min_db_ < 0.f) {
            return;
        }
        drawFreqs(g);
        drawDBs(g);
    }

    void FFTBackgroundPanel::drawFreqs(juce::Graphics& g) const {
        const auto bound = getLocalBounds().toFloat();
        // draw freq grid
        const auto thickness = base_.getFontSize() * 0.1f;
        juce::RectangleList<float> rect_list;
        for (const auto& freq : kFreqValues) {
            const auto p = std::log(static_cast<double>(freq) * .1) / std::log(freq_max_ * .1);
            const auto rect = juce::Rectangle(static_cast<float>(p) * bound.getWidth() - thickness * .5f, 0.f,
                                              thickness, bound.getHeight());
            if (rect.getRight() > bound.getWidth()) {
                break;
            }
            rect_list.add(rect);
        }
        g.setColour(grid_colour_);
        g.fillRectList(rect_list);
        // draw top and bottom gradient
        juce::ColourGradient gradient;
        gradient.point1 = juce::Point<float>(bound.getX(), bound.getY());
        gradient.point2 = juce::Point<float>(bound.getX(), bound.getBottom());
        gradient.isRadial = false;
        gradient.clearColours();
        gradient.addColour(0.0, base_.getBackgroundColour().withAlpha(1.f));
        gradient.addColour(base_.getFontSize() / bound.getHeight(),
                           base_.getBackgroundColour().withAlpha(0.f));
        gradient.addColour(1.f - 2.f * base_.getFontSize() / bound.getHeight(),
                           base_.getBackgroundColour().withAlpha(0.f));
        gradient.addColour(1.f - base_.getFontSize() / bound.getHeight(),
                           base_.getBackgroundColour().withAlpha(1.f));
        gradient.addColour(1.0, base_.getBackgroundColour().withAlpha(1.f));
        g.setGradientFill(gradient);
        g.fillRect(bound);
        // draw freq labels
        g.setColour(base_.getTextColour().withAlpha(.5f));
        g.setFont(base_.getFontSize() * 1.25f);
        const auto label_y0 = bound.getBottom() - base_.getFontSize() * 1.15f;
        const auto label_height = base_.getFontSize() * 1.1f;
        for (const auto& freq : kFreqValues) {
            const auto label = freq < 1000.f
                ? juce::String(static_cast<int>(freq))
                : juce::String(static_cast<int>(std::round(freq * 0.001f))) + "K";
            const auto label_width = freq < 20000.f
                ? base_.getFontSize() * 3.f
                : juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), label) * 1.1f;
            const auto p = std::log(static_cast<double>(freq) * .1) / std::log(freq_max_ * .1);
            const auto rect = juce::Rectangle(static_cast<float>(p) * bound.getWidth() - label_width * .5f, label_y0,
                                              label_width, label_height);
            if (rect.getRight() > bound.getWidth()) {
                break;
            }
            g.drawText(freq < 1000.f ? juce::String(freq) : juce::String(std::round(freq * 0.001f)) + "K",
                       rect, juce::Justification::centredBottom, false);
        }
    }

    void FFTBackgroundPanel::drawDBs(juce::Graphics& g) const {
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
        const auto min_db_unit = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(
            c_fft_min_db_))] / 6.f;
        for (size_t i = 1; i < 7; ++i) {
            const auto db = min_db_unit * static_cast<float>(i);
            const auto y = unit_height * static_cast<float>(i);
            const auto rect = juce::Rectangle(label_x0, y - label_height * .5f,
                                              label_width, label_height);
            g.drawText(juce::String(juce::roundToInt(db)),
                       rect, juce::Justification::centredRight, false);
        }
    }

    void FFTBackgroundPanel::updateSampleRate(const double sample_rate) {
        const auto freq_max = sample_rate * 0.5;
        if (std::abs(freq_max - freq_max_) > 0.1) {
            freq_max_ = freq_max;
            repaint();
        }
    }

    void FFTBackgroundPanel::repaintCallBackSlow() {
        const auto fft_min_db = fft_min_db_.load(std::memory_order::relaxed);
        if (std::abs(fft_min_db - c_fft_min_db_) > 0.1f) {
            c_fft_min_db_ = fft_min_db;
            repaint();
        }
    }

    void FFTBackgroundPanel::lookAndFeelChanged() {
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
