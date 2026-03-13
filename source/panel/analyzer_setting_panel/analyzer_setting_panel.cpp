// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "analyzer_setting_panel.hpp"

namespace zlpanel {
    AnalyzerSettingPanel::AnalyzerSettingPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        analyzer_type_idx_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerShow::kID)),
        pop_panel_(p, base) {
        pop_panel_.setBufferedToImage(true);
        addAndMakeVisible(pop_panel_);

        setInterceptsMouseClicks(false, true);

        base_.getPanelValueTree().addListener(this);
    }

    AnalyzerSettingPanel::~AnalyzerSettingPanel() {
        base_.getPanelValueTree().removeListener(this);
    }

    int AnalyzerSettingPanel::getIdealWidth() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto slider_width = getSliderWidth(font_size);

        return padding * 4 + slider_width * 2;
    }

    int AnalyzerSettingPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        const auto button_size = getButtonSize(font_size);

        return padding * 5 + button_size * 3;
    }

    void AnalyzerSettingPanel::resized() {
        const auto ideal_height = pop_panel_.getIdealHeight();
        const auto bound = juce::Rectangle{0, 0, getWidth(), ideal_height};
        if (bound != pop_panel_.getBounds()) {
            pop_panel_.setBounds(bound);
        } else {
            pop_panel_.resized();
        }
    }

    void AnalyzerSettingPanel::repaintCallBackSlow() {
        if (!isVisible()) {
            return;
        }
        const auto analyzer_type =
            static_cast<size_t>(std::round(analyzer_type_idx_.load(std::memory_order::relaxed)));
        if (analyzer_type != analyzer_type_) {
            analyzer_type_ = analyzer_type;
            pop_panel_.setAnalyzerType(analyzer_type);
        }
        pop_panel_.repaintCallBackSlow();
    }

    void AnalyzerSettingPanel::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& identifier) {
        if (base_.isPanelIdentifier(zlgui::PanelSettingIdx::kAnalyzerPanel, identifier)) {
            setVisible(static_cast<float>(base_.getPanelProperty(zlgui::PanelSettingIdx::kAnalyzerPanel)) > .5f);
        }
    }
}
