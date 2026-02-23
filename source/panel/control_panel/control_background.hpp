// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/gui.hpp"
#include "../helper/helper.hpp"

namespace zlpanel {
    class ControlBackground final : public juce::Component {
    public:
        explicit ControlBackground(zlgui::UIBase& base, float alpha = .5f);

        void paint(juce::Graphics& g) override;

    private:
        zlgui::UIBase& base_;
        const float alpha_;
    };
}
