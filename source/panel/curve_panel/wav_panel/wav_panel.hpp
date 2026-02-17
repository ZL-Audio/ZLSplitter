// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "wav_analyzer_panel.hpp"

namespace zlpanel {
    class WavPanel final : public juce::Component {
    public:
        explicit WavPanel(PluginProcessor &p, zlgui::UIBase &base);

        void run(double next_time_stamp);

        void resized() override;

        void repaintCallBackSlow();

    private:
        WavAnalyzerPanel wav_analyzer_panel_;
    };
}
