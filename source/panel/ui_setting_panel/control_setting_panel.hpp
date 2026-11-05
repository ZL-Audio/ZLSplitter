// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../state/ui_state.hpp"

#include "../../gui/interface_definitions.hpp"

#include "../../gui/gui.hpp"
#include "../../PluginProcessor.hpp"
#include "../helper/panel_constants.hpp"

namespace zlpanel {
    class ControlSettingPanel final : public juce::Component {
    public:
        static constexpr float kHeightP = 20.f;

        explicit ControlSettingPanel(PluginProcessor& p, zlgui::UIState& base);

        ~ControlSettingPanel() override;

        void loadSetting();

        void saveSetting();

        void resetSetting();

        int getIdealHeight() const;

        void resized() override;

    private:
        PluginProcessor& p_ref_;
        zlgui::UIState& base_;
        zlgui::label::NameLookAndFeel name_laf_;

        juce::Label wheel_label_;
        juce::Label slider_label_;
        juce::Label dragger_label_;
        std::array<zlgui::slider::CompactLinearSlider<true, true, true>, zlgui::kSensitivityNum> sensitivity_sliders_;
        zlgui::combobox::CompactCombobox wheel_reverse_box_;
        juce::Label rotary_style_label_;
        zlgui::combobox::CompactCombobox rotary_style_box_;
        juce::Label slider_double_click_label_;
        zlgui::combobox::CompactCombobox slider_double_click_box_;

    };
} // zlpanel
