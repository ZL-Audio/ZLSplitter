// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "analyzer_setting_pop_panel.hpp"

namespace zlpanel {
    class AnalyzerSettingPanel final : public juce::Component,
                                       private juce::ValueTree::Listener {
    public:
        explicit AnalyzerSettingPanel(PluginProcessor& p, zlgui::UIBase& base);

        ~AnalyzerSettingPanel() override;

        int getIdealWidth() const;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase& base_;
        std::atomic<float>& analyzer_type_idx_;
        size_t analyzer_type_{0};

        AnalyzerSettingPopPanel pop_panel_;

        void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& identifier) override;
    };
}
