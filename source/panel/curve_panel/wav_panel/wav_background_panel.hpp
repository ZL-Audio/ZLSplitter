// Copyright (C) 2026 - zsliu98
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
#include "../../../state/state.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class WavBackgroundPanel final : public juce::Component {
    public:
        explicit WavBackgroundPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase& base_;
        std::atomic<float>& max_db_ref_;

        int c_max_db_{0};

        juce::Colour grid_colour_;

        static constexpr std::array kDBs = {-1.f, -2.f, -4.f, -8.f, -16.f, -32.f, -64.f};

        void drawDBs(juce::Graphics& g) const;

        void lookAndFeelChanged() override;
    };
}
