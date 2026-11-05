// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../gui/interface_definitions.hpp"

#include <array>
#include <functional>

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlpanel {
    class UISettingTabBar final : public juce::Component {
    public:
        explicit UISettingTabBar(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        void mouseMove(const juce::MouseEvent& event) override;

        void mouseExit(const juce::MouseEvent& event) override;

        void mouseDown(const juce::MouseEvent& event) override;

        void mouseUp(const juce::MouseEvent& event) override;

        bool keyPressed(const juce::KeyPress& key) override;

        void setSelectedIndex(int index);

        std::function<void(int)> onTabSelected;

    private:
        zlgui::UIBase& base_;
        const std::array<juce::String, 4> tab_names_{"Colour", "Control", "Other", "Credit"};
        int selected_index_{0};
        int hovered_index_{-1};
        int mouse_down_index_{-1};

        [[nodiscard]] juce::Rectangle<int> getTabBounds(int index) const;

        [[nodiscard]] int getTabAt(juce::Point<int> position) const;

        void selectTab(int index, bool send_notification);
    };
}
