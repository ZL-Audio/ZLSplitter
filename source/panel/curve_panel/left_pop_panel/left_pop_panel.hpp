// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 8/20/25.
//

#pragma once

#include "lr_pop_panel.hpp"
#include "lh_pop_panel.hpp"
#include "ts_pop_panel.hpp"
#include "ps_pop_panel.hpp"

namespace zlpanel {
    class LeftPopPanel final : public juce::Component {
    public:
        explicit LeftPopPanel(PluginProcessor &p, zlgui::UIBase &base,
                              multilingual::TooltipHelper &tooltip_helper);

        void paint(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase &base_;
        juce::Path background_;

        std::atomic<float> &split_type_ref_;
        float c_split_type_{-1.f};

        LRPopPanel lr_pop_panel_;
        LHPopPanel lh_pop_panel_;
        TSPopPanel ts_pop_panel_;
        PSPopPanel ps_pop_panel_;
    };
}