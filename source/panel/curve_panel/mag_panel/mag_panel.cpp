// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "mag_panel.hpp"

namespace zlpanel {
    MagPanel::MagPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        mag_background_panel_(p, base),
        mag_analyzer_panel_(p, base) {
        addAndMakeVisible(mag_background_panel_);
        addAndMakeVisible(mag_analyzer_panel_);

        setInterceptsMouseClicks(false, true);
    }

    void MagPanel::run(const double next_time_stamp) {
        mag_analyzer_panel_.run(next_time_stamp);
    }

    void MagPanel::resized() {
        mag_background_panel_.setBounds(getLocalBounds());
        mag_analyzer_panel_.setBounds(getLocalBounds());
    }

    void MagPanel::repaintCallBackSlow() {
        mag_background_panel_.repaintCallBackSlow();
    }
}
