// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "wav_setting_panel.hpp"

namespace zlpanel {
    WavSettingPanel::WavSettingPanel(PluginProcessor& p, zlgui::UIState& base) :
        base_(base), updater_(),
        wav_max_db_box_(zlstate::PWavMaxDB::kChoices, base.gui),
        wav_max_db_attach_(wav_max_db_box_.getBox(), p.na_parameters_,
                           zlstate::PWavMaxDB::kID, updater_),
        mag_length_box_(zlstate::PMagTimeLength::kChoices, base.gui),
        mag_length_attach_(mag_length_box_.getBox(), p.na_parameters_,
                           zlstate::PMagTimeLength::kID, updater_),
        label_laf_(base.gui) {
        lookAndFeelChanged();
        wav_max_db_box_.setComponentID(zlstate::PWavMaxDB::kID);
        wav_max_db_box_.getBox().setComponentID(zlstate::PWavMaxDB::kID);
        mag_length_box_.setComponentID(zlstate::PMagTimeLength::kID);
        mag_length_box_.getBox().setComponentID(zlstate::PMagTimeLength::kID);
        wav_max_db_box_.setScrollEnabled(true);
        addAndMakeVisible(wav_max_db_box_);
        mag_length_box_.setScrollEnabled(true);
        addAndMakeVisible(mag_length_box_);

        labels[0].setText("Max dB", juce::dontSendNotification);
        labels[1].setText("Length", juce::dontSendNotification);

        label_laf_.setFontScale(1.5f);
        for (auto& label : labels) {
            label.setJustificationType(juce::Justification::centredRight);
            label.setLookAndFeel(&label_laf_);
            label.setBufferedToImage(true);
            addAndMakeVisible(label);
        }
    }

    WavSettingPanel::~WavSettingPanel() {
        for (auto& label : labels) {
            label.setLookAndFeel(nullptr);
        }
    }

    int WavSettingPanel::getIdealHeight() const {
        const auto font_size = base_.gui.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);
        return (padding + button_size) * static_cast<int>(labels.size());
    }

    void WavSettingPanel::resized() {
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
        wav_max_db_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        mag_length_box_.setBounds(bound.removeFromTop(button_size));
    }

    void WavSettingPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }

    void WavSettingPanel::lookAndFeelChanged() {
        wav_max_db_box_.getLAF().setAlignment(zlgui::combobox::Alignment::kRight);
        mag_length_box_.getLAF().setAlignment(zlgui::combobox::Alignment::kRight);
    }

}
