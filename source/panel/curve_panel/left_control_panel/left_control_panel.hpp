// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "BinaryData.h"

namespace zlpanel {
    class LeftControlPanel final : public juce::Component {
    public:
        explicit LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBackSlow();

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;

        std::atomic<float> &split_type_ref_;
        float c_split_type_{-1.f};

        std::array<std::unique_ptr<juce::Drawable>, 6> split_mode_drawables_;
        std::array<zlgui::button::CompactButton, 6> split_mode_buttons_;
    };
}
