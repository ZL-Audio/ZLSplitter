// Copyright (C) 2025 - zsliu98
// This file is part of ZLCompressor
//
// ZLCompressor is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLCompressor is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLCompressor. If not, see <https://www.gnu.org/licenses/>.

#include "other_ui_setting_panel.hpp"

namespace zlpanel {
    OtherUISettingPanel::OtherUISettingPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p),
          base_(base), name_laf_(base),
          refresh_rate_box_(zlstate::PTargetRefreshSpeed::kChoices, base),
          fft_tilt_slider_("Tilt", base),
          fft_speed_slider_("Speed", base),
          mag_curve_slider_("Mag", base),
          eq_curve_slider_("EQ", base),
          tooltip_box_(zlstate::PTooltipLang::kChoices, base) {
        juce::ignoreUnused(p_ref_);
        name_laf_.setFontScale(zlgui::kFontHuge);

        refresh_rate_label_.setText("Refresh Rate", juce::dontSendNotification);
        refresh_rate_label_.setJustificationType(juce::Justification::centredRight);
        refresh_rate_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(refresh_rate_label_);
        addAndMakeVisible(refresh_rate_box_);

        fft_label_.setText("FFT", juce::dontSendNotification);
        fft_label_.setJustificationType(juce::Justification::centredRight);
        fft_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(fft_label_);
        fft_tilt_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(-4.5, 4.5, .01));
        fft_tilt_slider_.getSlider().setDoubleClickReturnValue(true, 0.);
        addAndMakeVisible(fft_tilt_slider_);
        fft_speed_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0., 2., .01));
        fft_speed_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        addAndMakeVisible(fft_speed_slider_);

        curve_thick_label_.setText("Curve Thickness", juce::dontSendNotification);
        curve_thick_label_.setJustificationType(juce::Justification::centredRight);
        curve_thick_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(curve_thick_label_);
        mag_curve_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0., 4., .01));
        mag_curve_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        addAndMakeVisible(mag_curve_slider_);
        eq_curve_slider_.getSlider().setNormalisableRange(juce::NormalisableRange<double>(0., 4., .01));
        eq_curve_slider_.getSlider().setDoubleClickReturnValue(true, 1.0);
        addAndMakeVisible(eq_curve_slider_);

        tooltip_label_.setText("Tooltip", juce::dontSendNotification);
        tooltip_label_.setJustificationType(juce::Justification::centredRight);
        tooltip_label_.setLookAndFeel(&name_laf_);
        addAndMakeVisible(tooltip_label_);
        addAndMakeVisible(tooltip_box_);
    }

    void OtherUISettingPanel::loadSetting() {
        refresh_rate_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getRefreshRateID()));
        fft_tilt_slider_.getSlider().setValue(static_cast<double>(base_.getFFTExtraTilt()));
        fft_speed_slider_.getSlider().setValue(static_cast<double>(base_.getFFTExtraSpeed()));
        mag_curve_slider_.getSlider().setValue(base_.getMagCurveThickness());
        tooltip_box_.getBox().setSelectedItemIndex(static_cast<int>(base_.getTooltipLangID()));
        eq_curve_slider_.getSlider().setValue(base_.getEQCurveThickness());
    }

    void OtherUISettingPanel::saveSetting() {
        base_.setRefreshRateID(static_cast<size_t>(refresh_rate_box_.getBox().getSelectedItemIndex()));
        base_.setFFTExtraTilt(static_cast<float>(fft_tilt_slider_.getSlider().getValue()));
        base_.setFFTExtraSpeed(static_cast<float>(fft_speed_slider_.getSlider().getValue()));
        base_.setMagCurveThickness(static_cast<float>(mag_curve_slider_.getSlider().getValue()));
        base_.setEQCurveThickness(static_cast<float>(eq_curve_slider_.getSlider().getValue()));
        base_.setTooltipLandID(static_cast<size_t>(tooltip_box_.getBox().getSelectedItemIndex()));
        base_.saveToAPVTS();
    }

    void OtherUISettingPanel::resetSetting() {
    }

    int OtherUISettingPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        return padding * 5 + slider_height * 4;
    }

    void OtherUISettingPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale * 3.f);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        auto bound = getLocalBounds(); {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            refresh_rate_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            refresh_rate_box_.setBounds(local_bound.removeFromLeft(slider_width));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            fft_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            fft_tilt_slider_.setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            fft_speed_slider_.setBounds(local_bound.removeFromLeft(slider_width));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            curve_thick_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            mag_curve_slider_.setBounds(local_bound.removeFromLeft(slider_width));
            local_bound.removeFromLeft(padding);
            eq_curve_slider_.setBounds(local_bound.removeFromLeft(slider_width));
        } {
            bound.removeFromTop(padding);
            auto local_bound = bound.removeFromTop(slider_height);
            tooltip_label_.setBounds(local_bound.removeFromLeft(slider_width * 2));
            local_bound.removeFromLeft(padding);
            tooltip_box_.setBounds(local_bound.removeFromLeft(slider_width));
        }
    }
} // zlpanel
