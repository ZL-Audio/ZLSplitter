// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "fft_setting_panel.hpp"

namespace zlpanel {
    FFTSettingPanel::FFTSettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base), updater_(),
        fft_min_db_box_(zlstate::PFFTMinDB::kChoices, base_),
        fft_min_db_attach_(fft_min_db_box_.getBox(), p.na_parameters_,
                           zlstate::PFFTMinDB::kID, updater_),
        fft_smooth_box_(zlstate::PFFTSmooth::kChoices, base_),
        fft_smooth_attach_(fft_smooth_box_.getBox(), p.na_parameters_,
                           zlstate::PFFTSmooth::kID, updater_),
        fft_speed_box_(zlstate::PFFTSpeed::kChoices, base_),
        fft_speed_attach_(fft_speed_box_.getBox(), p.na_parameters_,
                          zlstate::PFFTSpeed::kID, updater_),
        fft_tilt_box_(zlstate::PFFTTilt::kChoices, base_),
        fft_tilt_attach_(fft_tilt_box_.getBox(), p.na_parameters_,
                         zlstate::PFFTTilt::kID, updater_),
        fft_order_box_(zlstate::PFFTOrder::kChoices, base_),
        fft_order_attach_(fft_order_box_.getBox(), p.na_parameters_,
                          zlstate::PFFTOrder::kID, updater_),
        label_laf_(base) {
        addAndMakeVisible(fft_min_db_box_);
        addAndMakeVisible(fft_smooth_box_);
        addAndMakeVisible(fft_speed_box_);
        addAndMakeVisible(fft_tilt_box_);
        addAndMakeVisible(fft_order_box_);

        labels[0].setText("Min dB", juce::dontSendNotification);
        labels[1].setText("Smooth", juce::dontSendNotification);
        labels[2].setText("Speed", juce::dontSendNotification);
        labels[3].setText("Tilt", juce::dontSendNotification);
        labels[4].setText("Order", juce::dontSendNotification);

        for (auto& label : labels) {
            label.setJustificationType(juce::Justification::centredRight);
            label.setBufferedToImage(true);
            addAndMakeVisible(label);
        }

        setInterceptsMouseClicks(false, true);
    }

    int FFTSettingPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);
        return (padding + button_size) * static_cast<int>(labels.size());
    }

    void FFTSettingPanel::resized() {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);
        auto bound = getLocalBounds();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - padding, bound.getHeight() - padding);
        auto text_bound = bound.removeFromLeft(bound.getWidth() / 2);
        text_bound.removeFromRight(padding);
        for (auto& label : labels) {
            label.setBounds(text_bound.removeFromTop(button_size));
            text_bound.removeFromTop(padding);
        }
        fft_min_db_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        fft_smooth_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        fft_speed_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        fft_tilt_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        fft_order_box_.setBounds(bound.removeFromTop(button_size));
    }

    void FFTSettingPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
