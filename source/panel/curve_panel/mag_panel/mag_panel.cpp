// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "mag_panel.hpp"

namespace zlpanel {
    MagPanel::MagPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), mag_analyzer_(p, base) {
        addAndMakeVisible(mag_analyzer_);

        setInterceptsMouseClicks(false, true);
    }

    void MagPanel::run(double next_time_stamp) {
        mag_analyzer_.run(next_time_stamp);
    }

    void MagPanel::resized() {
        mag_analyzer_.setBounds(getLocalBounds());
    }

    void MagPanel::visibilityChanged() {
        p_ref_.getController().setMagAnalyzerOn(isVisible());
    }
}
