// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    class TopLegendPanel final : public juce::Component {
    public:
        explicit TopLegendPanel(PluginProcessor &p, zlgui::UIBase &base);

        int getIdealWidth() const;

        void paint(juce::Graphics &g) override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase &base_;
        std::atomic<float> &split_type_ref_, &swap_ref_;
        float c_split_type_{static_cast<float>(zlp::PSplitType::kDefaultI)};
        bool c_swap_{zlp::PSwap::kDefaultV};

        static constexpr std::array kText1 = {
            "Left", "Mid", "Low", "Transient", "Peak", "None"
        };
        static constexpr std::array kText2 = {
            "Right", "Side", "High", "Steady", "Steady", "None"
        };
    };
}