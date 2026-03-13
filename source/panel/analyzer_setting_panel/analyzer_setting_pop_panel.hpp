// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../control_panel/control_background.hpp"
#include "fft_setting_panel.hpp"

namespace zlpanel {
    class AnalyzerSettingPopPanel final : public juce::Component {
    public:
        explicit AnalyzerSettingPopPanel(PluginProcessor& p, zlgui::UIBase& base);

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

        void setAnalyzerType(size_t idx);

    private:
        zlgui::UIBase& base_;
        size_t analyzer_type_{0};

        ControlBackground background_;
        FFTSettingPanel fft_setting_panel_;
    };
}
