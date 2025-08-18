// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class FFTAnalyzerPanel final : public juce::Component {
    public:
        explicit FFTAnalyzerPanel(PluginProcessor &processor, zlgui::UIBase &base);

        ~FFTAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void run();

        void resized() override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        std::atomic<float> &split_type_ref_, &swap_ref_;

        bool skip_next_repaint_{false};
        AtomicBound<float> atomic_bound_;
        float width_{-.1f};

        std::vector<float> xs_{};
        std::vector<float> y1_{}, y2_{};
        juce::Path out_path1_, next_out_path1_;
        juce::Path out_path2_, next_out_path2_;
        std::mutex mutex_;
    };
}
