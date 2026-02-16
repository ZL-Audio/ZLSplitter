// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ui_setting_panel.hpp"
#include "BinaryData.h"

namespace zlpanel {
    UISettingPanel::UISettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p), base_(base),
        colour_panel_(p, base),
        control_panel_(p, base),
        other_panel_(p, base),
        save_drawable_(juce::Drawable::createFromImageData(BinaryData::save_svg, BinaryData::save_svgSize)),
        close_drawable_(juce::Drawable::createFromImageData(BinaryData::close_svg, BinaryData::close_svgSize)),
        reset_drawable_(
            juce::Drawable::createFromImageData(BinaryData::reset_settings_svg, BinaryData::reset_settings_svgSize)),
        save_button_(base_, save_drawable_.get()),
        close_button_(base_, close_drawable_.get()),
        reset_button_(base_, reset_drawable_.get()),
        panel_name_laf_(base_),
        label_laf_(base_) {
        juce::ignoreUnused(p_ref_);
        setOpaque(true);
        base_.setPanelProperty(zlgui::PanelSettingIdx::kUISettingPanel, false);
        addAndMakeVisible(save_button_);
        addAndMakeVisible(close_button_);
        addAndMakeVisible(reset_button_);
        view_port_.setScrollBarsShown(true, false,
                                      true, false);
        changeDisplayPanel();
        addAndMakeVisible(view_port_);
        save_button_.getButton().onClick = [this]() {
            switch (current_panel_idx_) {
            case kColourP: {
                colour_panel_.saveSetting();
                break;
            }
            case kControlP: {
                control_panel_.saveSetting();
                break;
            }
            case kOtherP: {
                other_panel_.saveSetting();
                break;
            }
            }
            base_.setPanelProperty(zlgui::kUISettingChanged,
                                   !static_cast<bool>(base_.getPanelProperty(zlgui::kUISettingChanged)));
        };
        reset_button_.getButton().onClick = [this]() {
            switch (current_panel_idx_) {
            case kColourP: {
                colour_panel_.resetSetting();
                break;
            }
            case kControlP: {
                control_panel_.resetSetting();
                break;
            }
            case kOtherP: {
                other_panel_.resetSetting();
                break;
            }
            }
        };
        close_button_.getButton().onClick = [this]() {
            base_.setPanelProperty(zlgui::PanelSettingIdx::kUISettingPanel, false);
        };

        panel_name_laf_.setFontScale(1.5f);
        panel_labels_[0].setText("Colour", juce::dontSendNotification);
        panel_labels_[1].setText("Control", juce::dontSendNotification);
        panel_labels_[2].setText("Other", juce::dontSendNotification);
        for (auto& pL : panel_labels_) {
            pL.setInterceptsMouseClicks(true, false);
            pL.addMouseListener(this, false);
            pL.setJustificationType(juce::Justification::centred);
            pL.setLookAndFeel(&panel_name_laf_);
            addAndMakeVisible(pL);
        }

        label_laf_.setFontScale(1.125f);
        label_laf_.setAlpha(.5f);
        version_label_.setText(
            juce::String(ZL_PLUGIN_CURRENT_VERSION) + " " + juce::String(ZL_PLUGIN_CURRENT_HASH),
            juce::dontSendNotification);
        version_label_.setJustificationType(juce::Justification::bottomLeft);
        version_label_.setLookAndFeel(&label_laf_);
        version_label_.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(version_label_);
    }

    UISettingPanel::~UISettingPanel() = default;

    void UISettingPanel::paint(juce::Graphics& g) {
        g.fillAll(base_.getBackgroundColor());
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth() * .75f, bound.getHeight() * 1.25f);
        base_.fillRoundedShadowRectangle(g, bound, 0.5f * base_.getFontSize(), {.blur_radius = 0.5f});
    }

    void UISettingPanel::resized() {
        const auto button_width = juce::roundToInt(base_.getFontSize() * kButtonScale);
        auto bound = getLocalBounds();
        bound = bound.withSizeKeepingCentre(juce::roundToInt(static_cast<float>(bound.getWidth()) * .75f),
                                            bound.getHeight());
        {
            auto label_bound = bound.removeFromTop(button_width);
            const auto label_width = label_bound.getWidth() / static_cast<int>(panel_labels_.size());
            for (auto& panel_label : panel_labels_) {
                panel_label.setBounds(label_bound.removeFromLeft(label_width));
            }
        }

        colour_panel_.setBounds(0, 0,
                                juce::roundToInt(bound.getWidth()),
                                colour_panel_.getIdealHeight());
        control_panel_.setBounds(0, 0,
                                 juce::roundToInt(bound.getWidth()),
                                 control_panel_.getIdealHeight());
        other_panel_.setBounds(0, 0,
                               juce::roundToInt(bound.getWidth()),
                               other_panel_.getIdealHeight());

        view_port_.setBounds(bound.removeFromTop(bound.getHeight() - button_width - button_width / 2));

        bound = bound.withSizeKeepingCentre(bound.getWidth(), button_width);
        const auto left_bound = bound.removeFromLeft(bound.getWidth() / 3);
        const auto right_bound = bound.removeFromRight(bound.getWidth() / 2);
        const auto center_bound = bound;
        save_button_.setBounds(left_bound);
        reset_button_.setBounds(center_bound);
        close_button_.setBounds(right_bound);

        bound = getLocalBounds();
        bound = bound.removeFromBottom(button_width);
        bound.removeFromLeft(button_width / 10);
        bound.removeFromBottom(button_width / 10);
        version_label_.setBounds(bound);
    }

    void UISettingPanel::loadSetting() {
        colour_panel_.loadSetting();
        control_panel_.loadSetting();
        other_panel_.loadSetting();
    }

    void UISettingPanel::mouseDown(const juce::MouseEvent& event) {
        for (size_t i = 0; i < panel_labels_.size(); ++i) {
            if (event.originalComponent == &panel_labels_[i]) {
                current_panel_idx_ = static_cast<PanelIdx>(i);
                changeDisplayPanel();
                return;
            }
        }
    }

    void UISettingPanel::changeDisplayPanel() {
        switch (current_panel_idx_) {
        case kColourP: {
            view_port_.setViewedComponent(&colour_panel_, false);
            break;
        }
        case kControlP: {
            view_port_.setViewedComponent(&control_panel_, false);
            break;
        }
        case kOtherP: {
            view_port_.setViewedComponent(&other_panel_, false);
            break;
        }
        }
    }

    void UISettingPanel::visibilityChanged() {
        if (isVisible()) {
            colour_panel_.loadSetting();
            control_panel_.loadSetting();
            other_panel_.loadSetting();
        }
    }
} // zlpanel
