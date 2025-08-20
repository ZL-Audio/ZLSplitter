// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "top_panel.hpp"

namespace zlpanel {
    TopPanel::TopPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base), top_control_panel_(p, base) {
        addAndMakeVisible(top_control_panel_);
    }

    void TopPanel::paint(juce::Graphics &g) {
        g.fillAll(base_.getBackgroundColor());
    }

    int TopPanel::getIdealHeight() const {
        const auto button_width = juce::roundToInt(base_.getFontSize() * kButtonScale);
        return button_width;
    }

    void TopPanel::resized() {
        auto bound = getLocalBounds();
        top_control_panel_.setBounds(bound.removeFromTop(top_control_panel_.getIdealWidth()));
    }

    void TopPanel::repaintCallBackSlow() {
        top_control_panel_.repaintCallBackSlow();
    }
}
