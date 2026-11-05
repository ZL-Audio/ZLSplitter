// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "main_panel.hpp"

namespace zlpanel {
    MainPanel::MainPanel(PluginProcessor& p, zlgui::UIState& base,
                         const multilingual::TooltipLanguage language) :
        base_(base),
        tooltip_helper_(language),
        curve_panel_(p, base_, tooltip_helper_),
        control_panel_(p, base_, tooltip_helper_),
        top_panel_(p, base_, tooltip_helper_),
        analyzer_setting_panel_(p, base),
        ui_setting_panel_(p, base_),
        preset_browser_(p, base_, tooltip_helper_),
        tooltip_laf_(base_.gui), tooltip_window_(base_.gui, this),
        refresh_handler_(zlstate::PTargetRefreshSpeed::kRates[base_.getRefreshRateID()]) {
        juce::ignoreUnused(base_);
        addAndMakeVisible(curve_panel_);
        addAndMakeVisible(control_panel_);
        addAndMakeVisible(top_panel_);
        addChildComponent(analyzer_setting_panel_);
        addChildComponent(ui_setting_panel_);
        addChildComponent(preset_browser_);

        tooltip_window_.setLookAndFeel(&tooltip_laf_);
        tooltip_window_.setOpaque(false);
        tooltip_window_.setBufferedToImage(true);

        base_.getPanelValueTree().addListener(this);
    }

    MainPanel::~MainPanel() {
        tooltip_window_.setLookAndFeel(nullptr);
        base_.getPanelValueTree().removeListener(this);
    }

    void MainPanel::resized() {
        auto bound = getLocalBounds();

        const auto max_font_size = std::min(static_cast<float>(bound.getWidth()) * kFontSizeOverWidth,
                                            static_cast<float>(bound.getHeight()) / 26.f);
        const auto min_font_size = max_font_size * .25f;
        const auto font_size = base_.gui.getFontMode() == 0
            ? max_font_size * std::clamp(base_.gui.getFontScale(), .25f, 1.f)
            : std::clamp(base_.gui.getStaticFontSize(), min_font_size, max_font_size);
        base_.gui.setFontSize(font_size);

        const auto overlay_bound = getLocalBounds().reduced(2 * getPaddingSize(font_size));
        ui_setting_panel_.setBounds(overlay_bound.withSizeKeepingCentre(
            std::max(0, std::min(overlay_bound.getWidth(), ui_setting_panel_.getIdealWidth())),
            std::max(0, std::min(overlay_bound.getHeight(), ui_setting_panel_.getIdealHeight()))));
        preset_browser_.setBounds(overlay_bound.withSizeKeepingCentre(
            std::max(0, std::min(overlay_bound.getWidth(), preset_browser_.getIdealWidth())),
            std::max(0, std::min(overlay_bound.getHeight(), preset_browser_.getIdealHeight()))));

        top_panel_.setBounds(bound.removeFromTop(top_panel_.getIdealHeight()));
        curve_panel_.setBounds(bound);
        control_panel_.setBounds(bound.removeFromLeft(control_panel_.getIdealWidth()));
        analyzer_setting_panel_.setBounds(bound.removeFromRight(analyzer_setting_panel_.getIdealWidth()));
    }

    void MainPanel::repaintCallBack(const double time_stamp) {
        if (ui_setting_panel_.isVisible()) { ui_setting_panel_.flushPendingScroll(); }
        if (preset_browser_.isVisible()) { preset_browser_.flushPendingScroll(); }
        if (refresh_handler_.tick(time_stamp)) {
            if (time_stamp < previous_time_stamp_ || time_stamp - previous_time_stamp_ > 0.1) {
                previous_time_stamp_ = time_stamp;
                curve_panel_.repaintCallBackSlow();
                control_panel_.repaintCallBackSlow();
                analyzer_setting_panel_.repaintCallBackSlow();
                top_panel_.repaintCallBackSlow();
            }

            curve_panel_.repaintCallBack(time_stamp);

            const auto c_refresh_rate = refresh_handler_.getActualRefreshRate();
            if (std::abs(c_refresh_rate - refresh_rate_) > 1e-3) {
                refresh_rate_ = c_refresh_rate;
                curve_panel_.setRefreshRate(refresh_rate_);
            }
        }
    }

    void MainPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
        if (base_.isPanelIdentifier(zlgui::kUISettingChanged, property)) {
            refresh_handler_.setTargetRefreshRate(zlstate::PTargetRefreshSpeed::kRates[base_.getRefreshRateID()]);
        } else if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kUISettingPanel, property)) {
            const auto ui_setting_visibility = static_cast<bool>(base_.getPanelProperty(
                zlgui::PanelSettingIdx::kUISettingPanel));
            if (ui_setting_visibility) { base_.setPanelProperty(zlgui::kPresetBrowser, false); }
            ui_setting_panel_.setVisible(ui_setting_visibility);
            if (ui_setting_visibility) { ui_setting_panel_.toFront(false); }
        } else if (base_.isPanelIdentifier(zlgui::kPresetBrowser, property)) {
            if (static_cast<bool>(base_.getPanelProperty(zlgui::kPresetBrowser))) {
                base_.setPanelProperty(zlgui::kUISettingPanel, false);
                preset_browser_.toFront(false);
            }
        }
    }

}
