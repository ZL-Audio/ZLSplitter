// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "control_pop_panel.hpp"

namespace zlpanel {
    class ControlPanel final : public juce::Component {
    public:
        explicit ControlPanel(PluginProcessor& p, zlgui::UIBase& base,
                              multilingual::TooltipHelper& tooltip_helper);

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase& base_;
        std::atomic<float>& split_type_idx_;
        zlp::PSplitType::SplitType split_type_{zlp::PSplitType::SplitType::kNone};

        ControlPopPanel control_pop_panel_;
    };
}
