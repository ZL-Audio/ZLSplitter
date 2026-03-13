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
    WavSettingPanel::WavSettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base), updater_(),
        wav_max_db_box_(zlstate::PWavMaxDB::kChoices, base),
        wav_max_db_attach_(wav_max_db_box_.getBox(), p.na_parameters_,
                           zlstate::PWavMaxDB::kID, updater_),
        mag_length_box_(zlstate::PMagTimeLength::kChoices, base),
        mag_length_attach_(mag_length_box_.getBox(), p.na_parameters_,
                           zlstate::PMagTimeLength::kID, updater_),
        label_laf_(base) {
        addAndMakeVisible(wav_max_db_box_);
        addAndMakeVisible(mag_length_box_);

        labels[0].setText("Max dB", juce::dontSendNotification);
        labels[1].setText("Length", juce::dontSendNotification);

        for (auto& label : labels) {
            label.setJustificationType(juce::Justification::centredRight);
            label.setBufferedToImage(true);
            addAndMakeVisible(label);
        }
    }

    int WavSettingPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);
        return (padding + button_size) * static_cast<int>(labels.size());
    }

    void WavSettingPanel::resized() {
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
        wav_max_db_box_.setBounds(bound.removeFromTop(button_size));
        bound.removeFromTop(padding);
        mag_length_box_.setBounds(bound.removeFromTop(button_size));
    }

    void WavSettingPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
