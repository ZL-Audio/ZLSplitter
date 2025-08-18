// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../helper/panel_constants.hpp"

namespace zlpanel {
    class OtherUISettingPanel final : public juce::Component {
    public:
        explicit OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base);

        void loadSetting();

        void saveSetting();

        void resetSetting();

        int getIdealHeight() const;

        void resized() override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        zlgui::label::NameLookAndFeel name_laf_;

        juce::Label refresh_rate_label_;
        zlgui::combobox::CompactCombobox refresh_rate_box_;
        juce::Label fft_label_;
        zlgui::slider::CompactLinearSlider<true, true, true> fft_tilt_slider_, fft_speed_slider_;
        juce::Label curve_thick_label_;
        zlgui::slider::CompactLinearSlider<true, true, true> mag_curve_slider_, eq_curve_slider_;
        juce::Label tooltip_label_;
        zlgui::combobox::CompactCombobox tooltip_box_;
    };
} // zlpanel
