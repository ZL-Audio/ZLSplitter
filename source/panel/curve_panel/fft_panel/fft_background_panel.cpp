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
    FFTBackgroundPanel::Background1::Background1(zlgui::UIBase& base) :
        base_(base) {
        setBufferedToImage(true);
        setInterceptsMouseClicks(false, false);
    }

    void FFTBackgroundPanel::Background1::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColor());

        const auto bound = getLocalBounds().toFloat();
        const auto thickness = base_.getFontSize() * 0.1f;
        g.setFont(base_.getFontSize());

        const auto grid_colour = base_.getColourByIdx(zlgui::ColourIdx::kGridColour);
        const auto text_colour = base_.getTextColor().withAlpha(
            std::clamp(grid_colour.getFloatAlpha() + .2f, .2f, 1.f));
        for (size_t i = 0; i < kBackgroundFreqs.size(); ++i) {
            const auto p = std::log(kBackgroundFreqs[i] / min_freq_) / std::log(max_freq_ / min_freq_);
            const auto x = bound.getX() + static_cast<float>(p) * bound.getWidth();
            if (x > bound.getRight() - base_.getFontSize() * 4.f) {
                break;
            }
            g.setColour(grid_colour);
            g.fillRect(juce::Rectangle<float>{x - thickness * .5f, bound.getY(), thickness, bound.getHeight()});
            const auto text_bound = juce::Rectangle<float>(x - base_.getFontSize() * 3 - base_.getFontSize() * 0.125f,
                                                           bound.getBottom() - base_.getFontSize() * 2,
                                                           base_.getFontSize() * 3, base_.getFontSize() * 2);
            g.setColour(text_colour);
            g.drawText(std::string(kBackgroundFreqsNames[i]), text_bound, juce::Justification::bottomRight);
        }

        const auto right_padding = std::round(base_.getFontSize() * .25f);
        for (size_t i = 1; i < 6; ++i) {
            const auto y = bound.getY() + static_cast<float>(i) / 6.f * bound.getHeight();
            g.setColour(grid_colour);
            g.fillRect(juce::Rectangle<float>{bound.getX(), y - thickness * .5f, bound.getWidth(), thickness});
            const auto text_bound = juce::Rectangle<float>(bound.getRight() - base_.getFontSize() * 3 - right_padding,
                                                           y - base_.getFontSize() * 2,
                                                           base_.getFontSize() * 3, base_.getFontSize() * 2);
            g.setColour(text_colour);
            g.drawText(juce::String(juce::roundToInt(static_cast<float>(i) / 6.f * min_db_)),
                       text_bound, juce::Justification::bottomRight);
        }
    }

    FFTBackgroundPanel::FFTBackgroundPanel(PluginProcessor& processor, zlgui::UIBase& base) :
        base_(base), background1_(base),
        updater_(),
        fft_min_freq_box_(zlstate::PFFTMinFreq::kChoices, base),
        fft_min_freq_attach_(fft_min_freq_box_.getBox(), processor.na_parameters_,
                             zlstate::PFFTMinFreq::kID, updater_),
        fft_max_freq_box_(zlstate::PFFTMaxFreq::kChoices, base),
        fft_max_freq_attach_(fft_max_freq_box_.getBox(), processor.na_parameters_,
                             zlstate::PFFTMaxFreq::kID, updater_),
        fft_min_db_box_(zlstate::PFFTMinDB::kChoices, base),
        fft_min_db_attach_(fft_min_db_box_.getBox(), processor.na_parameters_,
                           zlstate::PFFTMinDB::kID, updater_) {
        addAndMakeVisible(background1_);

        const auto popup_option = juce::PopupMenu::Options().withPreferredPopupDirection(
            juce::PopupMenu::Options::PopupDirection::upwards);

        const auto box_alpha = std::clamp(
            base_.getColourByIdx(zlgui::ColourIdx::kGridColour).getFloatAlpha() + .2f, .2f, 1.f);

        fft_min_freq_box_.getLAF().setLabelJustification(juce::Justification::bottomLeft);
        fft_max_freq_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);
        fft_min_db_box_.getLAF().setLabelJustification(juce::Justification::bottomRight);

        for (auto& b : {&fft_min_freq_box_, &fft_max_freq_box_, &fft_min_db_box_}) {
            b->setAlpha(box_alpha);
            b->getLAF().setOption(popup_option);
            b->getLAF().setFontScale(1.f);
            addAndMakeVisible(b);
        }

        setBufferedToImage(true);
        setInterceptsMouseClicks(false, true);
    }

    void FFTBackgroundPanel::resized() {
        const auto bound = getLocalBounds();
        const auto box_height = juce::roundToInt(base_.getFontSize() * 1.25f);
        const auto box_width = juce::roundToInt(base_.getFontSize() * 2.5f);
        const auto padding1 = box_width / 10;
        const auto padding2 = juce::roundToInt(std::ceil(base_.getFontSize()));
        const auto padding3 = static_cast<int>(std::round(base_.getFontSize() * .25f));
        background1_.setBounds(bound);
        fft_min_freq_box_.setBounds({
            bound.getX() + padding1, bound.getBottom() - box_height,
            box_width, box_height
        });
        fft_max_freq_box_.setBounds({
            bound.getRight() - box_width - padding2, bound.getBottom() - box_height,
            box_width, box_height
        });
        fft_min_db_box_.setBounds({
            bound.getRight() - box_width - padding3, bound.getBottom() - box_height - padding2,
            box_width, box_height
        });
    }

    void FFTBackgroundPanel::repaintCallBackSlow() {
        updater_.updateComponents();
        if (fft_min_db_box_.getBox().getSelectedItemIndex() != c_min_db_index_) {
            c_min_db_index_ = fft_min_db_box_.getBox().getSelectedItemIndex();
            background1_.setMinDB(zlstate::PFFTMinDB::kDBs[static_cast<size_t>(c_min_db_index_)]);
            background1_.repaint();
        }
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
