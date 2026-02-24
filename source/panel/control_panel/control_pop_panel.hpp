// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "lr_pop_panel.hpp"
#include "lh_pop_panel.hpp"
#include "control_background.hpp"

namespace zlpanel {
    class ControlPopPanel final : public juce::Component {
    public:
        explicit ControlPopPanel(PluginProcessor& p, zlgui::UIBase& base,
                                 const multilingual::TooltipHelper& tooltip_helper);

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

        void setSplitType(zlp::PSplitType::SplitType split_type);

    private:
        zlgui::UIBase& base_;
        zlp::PSplitType::SplitType split_type_{zlp::PSplitType::SplitType::kNone};

        ControlBackground background_;
        LRPopPanel lr_pop_panel_;
        LHPopPanel lh_pop_panel_;
    };
}
