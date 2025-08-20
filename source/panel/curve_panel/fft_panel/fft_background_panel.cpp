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
        setBufferedToImage(true);
        setInterceptsMouseClicks(false, false);
    }

    void FFTBackgroundPanel::Background1::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());

        const auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.1f;
        g.setFont(base_.getFontSize());
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            const auto p = std::log(kBackgroundFreqs[i] / min_freq_) / std::log(max_freq_ / min_freq_);
            const auto x = bound.getX() + static_cast<float>(p) * bound.getWidth();
            if (x > bound.getRight() - base_.getFontSize() * 4.f) {
                break;
            }
            g.setColour(base_.getTextColor().withAlpha(.1f));
            g.fillRect(juce::Rectangle<float>{x - thickness * .5f, bound.getY(), thickness, bound.getHeight()});
            const auto text_bound = juce::Rectangle<float>(x - base_.getFontSize() * 3 - base_.getFontSize() * 0.125f,
                                                           bound.getBottom() - base_.getFontSize() * 2,
                                                           base_.getFontSize() * 3, base_.getFontSize() * 2);
            g.setColour(base_.getTextColor().withAlpha(.375f));
            g.drawText(kBackgroundFreqsNames[i], text_bound, juce::Justification::bottomRight);
        }
    }

    FFTBackgroundPanel::FFTBackgroundPanel(PluginProcessor &processor, zlgui::UIBase &base)
        : base_(base), background1_(base),
          updater_(),
          fft_min_freq_box_(zlstate::PFFTMinFreq::kChoices, base),
          fft_min_freq_attach(fft_min_freq_box_.getBox(), processor.na_parameters_,
                              zlstate::PFFTMinFreq::kID, updater_),
          fft_max_freq_box_(zlstate::PFFTMaxFreq::kChoices, base),
          fft_max_freq_attach(fft_max_freq_box_.getBox(), processor.na_parameters_,
                              zlstate::PFFTMaxFreq::kID, updater_) {
        addAndMakeVisible(background1_);

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);

        fft_min_freq_box_.setAlpha(.375f);
        fft_min_freq_box_.getLAF().setOption(popup_option);
        fft_min_freq_box_.getLAF().setFontScale(1.f);
        fft_min_freq_box_.getLAF().setLabelJustification(juce::Justification::bottomLeft);
        addAndMakeVisible(fft_min_freq_box_);

        fft_max_freq_box_.setAlpha(.375f);
        fft_max_freq_box_.getLAF().setOption(popup_option);
        fft_max_freq_box_.getLAF().setFontScale(1.f);
        fft_max_freq_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);
        addAndMakeVisible(fft_max_freq_box_);

        setBufferedToImage(true);
        setInterceptsMouseClicks(false, true);
    }

    void FFTBackgroundPanel::resized() {
        const auto bound = getLocalBounds();
        const auto box_height = juce::roundToInt(base_.getFontSize() * 1.25f);
        const auto box_width = juce::roundToInt(base_.getFontSize() * 2.f);
        const auto padding1 = box_width / 10;
        const auto padding2 = juce::roundToInt(std::ceil(base_.getFontSize()));
        background1_.setBounds(bound);
        fft_min_freq_box_.setBounds({
            bound.getX() + padding1, bound.getBottom() - box_height,
            box_width, box_height
        });
        fft_max_freq_box_.setBounds({
            bound.getRight() - box_width - padding2, bound.getBottom() - box_height,
            box_width, box_height
        });
    }

    void FFTBackgroundPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void FFTBackgroundPanel::setMinMaxFreq(const double min_freq, const double max_freq) {
        background1_.setMinFreq(min_freq);
        background1_.setMaxFreq(max_freq);
        background1_.repaint();

        fft_max_freq_box_.getLAF().setLabelString(juce::String(
            static_cast<int>(std::round(max_freq * 0.001))) + "k");
        fft_max_freq_box_.repaint();
    }
}
