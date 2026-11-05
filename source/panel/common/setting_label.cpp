// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "setting_label.hpp"

#include "../helper/panel_constants.hpp"

namespace zlpanel {
    SettingLabel::SettingLabel(PluginProcessor&, zlgui::UIState& base,
                               const juce::String label, const zlgui::PanelSettingIdx setting_idx,
                               const juce::String& tooltip_text) :
        base_(base), setting_idx_(setting_idx),
        control_background_(base.gui),
        label_laf_(base.gui),
        setting_label_("", label) {
        SettableTooltipClient::setTooltip(tooltip_text);

        control_background_.setInterceptsMouseClicks(false, false);
        addChildComponent(control_background_);

        label_laf_.setFontScale(1.5f);

        for (auto& l : {&setting_label_}) {
            l->setInterceptsMouseClicks(false, false);
            l->setLookAndFeel(&label_laf_);
            l->setJustificationType(juce::Justification::centred);
            l->setBufferedToImage(true);
            addAndMakeVisible(l);
        }

        setAlpha(.5f);
        setInterceptsMouseClicks(true, false);

        base_.getPanelValueTree().addListener(this);
    }

    SettingLabel::~SettingLabel() {
        base_.getPanelValueTree().removeListener(this);
    }

    void SettingLabel::resized() {
        const auto padding = 2 * getPaddingSize(base_.gui.getFontSize());
        const auto bound = getLocalBounds();
        control_background_.setBounds(0, -padding, bound.getWidth(), bound.getHeight() + padding + padding / 4);
        setting_label_.setBounds(bound);
    }

    void SettingLabel::mouseDown(const juce::MouseEvent&) {
        const auto f = static_cast<double>(base_.getPanelProperty(setting_idx_));
        base_.setPanelProperty(setting_idx_, f < .5 ? 1. : 0.);
    }

    void SettingLabel::mouseEnter(const juce::MouseEvent&) {
        is_over_ = true;
        const auto f = static_cast<double>(base_.getPanelProperty(setting_idx_));
        updateAlpha(f > .5);
    }

    void SettingLabel::mouseExit(const juce::MouseEvent&) {
        is_over_ = false;
        const auto f = static_cast<double>(base_.getPanelProperty(setting_idx_));
        updateAlpha(f > .5);
    }

    void SettingLabel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(setting_idx_, property)) {
            const auto f = static_cast<double>(base_.getPanelProperty(setting_idx_));
            control_background_.setVisible(f > .5);
            updateAlpha(f > .5);
        }
    }

    void SettingLabel::updateAlpha(const bool is_panel_open) {
        if (is_panel_open) {
            setAlpha(1.f);
        } else if (is_over_) {
            setAlpha(.75f);
        } else {
            setAlpha(.5f);
        }
    }
}
