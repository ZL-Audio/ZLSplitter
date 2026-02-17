// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "wav_panel.hpp"

namespace zlpanel {
    WavPanel::WavPanel(PluginProcessor& p, zlgui::UIBase& base) :
        wav_analyzer_panel_(p, base) {
        addAndMakeVisible(wav_analyzer_panel_);

        setInterceptsMouseClicks(false, true);
    }

    void WavPanel::run(const double next_time_stamp) {
        wav_analyzer_panel_.run(next_time_stamp);
    }

    void WavPanel::resized() {
        wav_analyzer_panel_.setBounds(getLocalBounds());
    }

    void WavPanel::repaintCallBackSlow() {

    }
}
