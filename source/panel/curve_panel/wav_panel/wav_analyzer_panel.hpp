// Copyright (C) 2026 - zsliu98
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
#include "../../../state/state.hpp"
#include "../../helper/helper.hpp"
#include "../../../dsp/analyzer/wave_analyzer/wave_receiver.hpp"
#include "../../../dsp/lock/spin_lock.hpp"

namespace zlpanel {
    class WavAnalyzerPanel final : public juce::Component {
    public:
        explicit WavAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~WavAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void run(double next_time_stamp);

        void resized() override;

    private:
        static constexpr std::array<int, 4> kNumPointsPerSecond{40, 30, 20, 15};
        static constexpr int kPausedThreshold = 6;
        static constexpr int kTooMuchResetThreshold = 64;
        PluginProcessor &p_ref_;
        zlgui::UIBase& base_;

        std::atomic<float> &split_type_ref_, &swap_ref_;
        std::atomic<float>& analyzer_time_length_ref_;

        AtomicBound<float> atomic_bound_;

        std::array<float, 2> minmax1_{0.f, 0.f}, minmax2_{0.f, 0.f};
        kfr::univector<float> xs_{}, min1s_{}, max1s_{}, min2s_{}, max2s_{};
        juce::Path path1_, path2_;
        juce::Path next_path1_, next_path2_;
        zldsp::lock::SpinLock mutex_;

        double start_time_{0.0};

        bool is_first_point_{true};
        int too_much_samples_{0};
        int num_missing_points_{0};

        double sample_rate_{0.};
        size_t max_num_samples_{0};
        float time_length_idx_{0.f}, time_length_{6.f};

        size_t num_points_{0};
        int num_samples_per_point_{0};
        int num_points_per_second_{0};
        double second_per_point_{0};

        void updatePaths(juce::Rectangle<float> bound);
    };
}
