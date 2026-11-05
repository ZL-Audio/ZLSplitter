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
    FFTSettingPanel::FFTSettingPanel(PluginProcessor& p, zlgui::UIState& base) :
        base_(base), updater_(),
        fft_min_db_box_(zlstate::PFFTMinDB::kChoices, base_.gui),
        fft_min_db_attach_(fft_min_db_box_.getBox(), p.na_parameters_,
                           zlstate::PFFTMinDB::kID, updater_),
        fft_smooth_oct_value_box_(zlstate::PFFTSmoothOCTValue::kChoices, base_.gui),
        fft_smooth_oct_value_attach_(fft_smooth_oct_value_box_.getBox(), p.na_parameters_,
                                    zlstate::PFFTSmoothOCTValue::kID, updater_),
        fft_smooth_erb_value_box_(zlstate::PFFTSmoothERBValue::kChoices, base_.gui),
        fft_smooth_erb_value_attach_(fft_smooth_erb_value_box_.getBox(), p.na_parameters_,
                                    zlstate::PFFTSmoothERBValue::kID, updater_),
        fft_smooth_type_box_(zlstate::PFFTSmoothType::kChoices, base_.gui),
        fft_smooth_type_attach_(fft_smooth_type_box_.getBox(), p.na_parameters_,
                               zlstate::PFFTSmoothType::kID, updater_),
        fft_speed_box_(zlstate::PFFTSpeed::kChoices, base_.gui),
        fft_speed_attach_(fft_speed_box_.getBox(), p.na_parameters_,
                          zlstate::PFFTSpeed::kID, updater_),
        fft_tilt_box_(zlstate::PFFTTilt::kChoices, base_.gui),
        fft_tilt_attach_(fft_tilt_box_.getBox(), p.na_parameters_,
                         zlstate::PFFTTilt::kID, updater_),
        label_laf_(base.gui) {
        lookAndFeelChanged();
        fft_min_db_box_.setComponentID(zlstate::PFFTMinDB::kID);
        fft_min_db_box_.getBox().setComponentID(zlstate::PFFTMinDB::kID);
        fft_smooth_oct_value_box_.setComponentID(zlstate::PFFTSmoothOCTValue::kID);
        fft_smooth_oct_value_box_.getBox().setComponentID(zlstate::PFFTSmoothOCTValue::kID);
        fft_smooth_erb_value_box_.setComponentID(zlstate::PFFTSmoothERBValue::kID);
        fft_smooth_erb_value_box_.getBox().setComponentID(zlstate::PFFTSmoothERBValue::kID);
        fft_smooth_type_box_.setComponentID(zlstate::PFFTSmoothType::kID);
        fft_smooth_type_box_.getBox().setComponentID(zlstate::PFFTSmoothType::kID);
        fft_speed_box_.setComponentID(zlstate::PFFTSpeed::kID);
        fft_speed_box_.getBox().setComponentID(zlstate::PFFTSpeed::kID);
        fft_tilt_box_.setComponentID(zlstate::PFFTTilt::kID);
        fft_tilt_box_.getBox().setComponentID(zlstate::PFFTTilt::kID);
        fft_min_db_box_.setScrollEnabled(true);
        addAndMakeVisible(fft_min_db_box_);
        for (auto* box : {&fft_smooth_oct_value_box_, &fft_smooth_erb_value_box_, &fft_smooth_type_box_}) {
            box->setScrollEnabled(true);
            addAndMakeVisible(box);
        }
        fft_smooth_type_box_.getBox().onChange = [this]() {
            const auto selected_index = fft_smooth_type_box_.getBox().getSelectedItemIndex();
            fft_smooth_oct_value_box_.setVisible(selected_index == 0);
            fft_smooth_erb_value_box_.setVisible(selected_index == 1);
        };
        fft_speed_box_.setScrollEnabled(true);
        addAndMakeVisible(fft_speed_box_);
        fft_tilt_box_.setScrollEnabled(true);
        addAndMakeVisible(fft_tilt_box_);
        labels[0].setText("Min dB", juce::dontSendNotification);
        labels[1].setText("Smooth", juce::dontSendNotification);
        labels[2].setText("Speed", juce::dontSendNotification);
        labels[3].setText("Tilt", juce::dontSendNotification);
        label_laf_.setFontScale(1.5f);
        for (auto& label : labels) {
            label.setJustificationType(juce::Justification::centredRight);
            label.setLookAndFeel(&label_laf_);
            label.setBufferedToImage(true);
            addAndMakeVisible(label);
        }

        setInterceptsMouseClicks(false, true);
        updater_.updateComponents();
    }

    FFTSettingPanel::~FFTSettingPanel() {
        for (auto& label : labels) {
            label.setLookAndFeel(nullptr);
        }
    }

    int FFTSettingPanel::getIdealHeight() const {
        const auto font_size = base_.gui.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);
        return (padding + button_size) * static_cast<int>(labels.size());
    }

    void FFTSettingPanel::resized() {
        const auto font_size = base_.gui.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);
        auto bound = getLocalBounds();
        bound = bound.withSizeKeepingCentre(bound.getWidth() - padding, bound.getHeight() - padding);
        auto text_bound = bound.removeFromLeft(bound.getWidth() / 2 - padding);
        text_bound.removeFromRight(padding);
        for (auto& label : labels) {
            label.setBounds(text_bound.removeFromTop(button_size));
            text_bound.removeFromTop(padding);
        }
        fft_min_db_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        auto smooth_bound = bound.removeFromTop(button_size);
        fft_smooth_type_box_.setBounds(smooth_bound.removeFromRight(smooth_bound.getWidth() / 2));
        fft_smooth_oct_value_box_.setBounds(smooth_bound);
        fft_smooth_erb_value_box_.setBounds(smooth_bound);
        bound.removeFromTop(padding);
        fft_speed_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        fft_tilt_box_.setBounds(bound.removeFromTop(button_size));
    }

    void FFTSettingPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void FFTSettingPanel::lookAndFeelChanged() {
        const auto alignment = combobox_helper::getAlignment(base_.gui.getComboboxAlignment());
        fft_smooth_type_box_.getLAF().setItemAlignment(alignment);
        fft_speed_box_.getLAF().setItemAlignment(alignment);
        fft_min_db_box_.getLAF().setAlignment(zlgui::combobox::Alignment::kRight);
        fft_tilt_box_.getLAF().setAlignment(zlgui::combobox::Alignment::kRight);
    }

}
