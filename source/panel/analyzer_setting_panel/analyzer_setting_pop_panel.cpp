// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "analyzer_setting_pop_panel.hpp"

namespace zlpanel {
    AnalyzerSettingPopPanel::AnalyzerSettingPopPanel(PluginProcessor& p, zlgui::UIBase& base) :
        base_(base),
        background_(base),
        fft_setting_panel_(p, base) {
        addAndMakeVisible(background_);
        addAndMakeVisible(fft_setting_panel_);
    }

    int AnalyzerSettingPopPanel::getIdealHeight() const {
        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        auto height = 2 * padding;
        if (analyzer_type_ == 0) {
            height += fft_setting_panel_.getIdealHeight();
        }
        return height;
    }

    void AnalyzerSettingPopPanel::resized() {
        auto bound = getLocalBounds();
        background_.setBounds(bound);

        const auto font_size = base_.getFontSize();
        const auto padding = getPaddingSize(font_size);
        bound.reduce(padding, padding);
        if (analyzer_type_ == 0) {
            fft_setting_panel_.setBounds(bound);
        }
    }

    void AnalyzerSettingPopPanel::repaintCallBackSlow() {
        if (analyzer_type_ == 0) {
            fft_setting_panel_.repaintCallBackSlow();
        }
        if (isMouseOver(true)) {
            setAlpha(1.f);
        } else {
            setAlpha(.5f);
        }
    }

    void AnalyzerSettingPopPanel::setAnalyzerType(const size_t idx) {
        analyzer_type_ = idx;
        fft_setting_panel_.setVisible(idx == 0);
    }
}
