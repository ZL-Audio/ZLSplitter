// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "fft_background_panel.hpp"

namespace zlpanel {
    FFTBackgroundPanel::Background1::Background1(zlgui::UIBase &base)
        : base_(base) {
        setInterceptsMouseClicks(false, false);
        setBufferedToImage(true);
    }

    void FFTBackgroundPanel::Background1::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor().withAlpha(.875f));
        g.setFont(base_.getFontSize());
        g.setColour(base_.getTextColor().withAlpha(.375f));
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            g.drawText(kBackgroundFreqsNames[i], text_bounds_[i], juce::Justification::bottomRight);
        }
        g.setColour(base_.getTextColor().withAlpha(.1f));
        g.fillRectList(rect_list_);
    }

    void FFTBackgroundPanel::Background1::resized() {
        rect_list_.clear();
        auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.1f;
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            const auto x = kBackgroundFreqs[i] * bound.getWidth() + bound.getX();
            rect_list_.add({x - thickness * .5f, bound.getY(), thickness, bound.getHeight()});
            text_bounds_[i] = juce::Rectangle<float>(x - base_.getFontSize() * 3 - base_.getFontSize() * 0.125f,
                                                   bound.getBottom() - base_.getFontSize() * 2,
                                                   base_.getFontSize() * 3, base_.getFontSize() * 2);
        }

        bound = bound.withSizeKeepingCentre(bound.getWidth(), bound.getHeight() - 2 * base_.getFontSize());

        for (auto &d: kBackgroundDBs) {
            const auto y = d * bound.getHeight() + bound.getY();
            rect_list_.add({bound.getX(), y - thickness * .5f, bound.getWidth(), thickness});
        }
    }

    FFTBackgroundPanel::FFTBackgroundPanel(PluginProcessor &, zlgui::UIBase &base)
        : background1_(base) {
        addAndMakeVisible(background1_);
        setInterceptsMouseClicks(false, false);
    }

    void FFTBackgroundPanel::resized() {
        background1_.setBounds(getLocalBounds());
    }
} // zlpanel
