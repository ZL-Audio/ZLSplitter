// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../state/ui_state.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../helper/helper.hpp"
#include "../../helper/history_timing.hpp"
#include "../../../chore/thread/tri_buffer.hpp"
#include "../../../dsp/analyzer/analyzer_base/fifo_transfer_buffer.hpp"
#include "../../../dsp/analyzer/mag_analyzer/mag_receiver.hpp"

namespace zlpanel {
    class MagAnalyzerPanel final : public juce::Component {
    public:
        explicit MagAnalyzerPanel(PluginProcessor& p, zlgui::UIState& base);

        ~MagAnalyzerPanel() override;

        void paint(juce::Graphics& g) override;

        void run(double next_time_stamp);

        void reset() { reset_requested_ = true; }

        void resized() override;

    private:
        static constexpr std::array<int, 4> kNumPointsPerSecond{40, 30, 20, 15};
        static constexpr int kPausedThreshold = 6;
        static constexpr int kTooMuchResetThreshold = 64;
        static constexpr float kMissingDB = -10000.f;
        PluginProcessor& p_ref_;
        zlgui::UIState& base_;

        std::atomic<float>& split_type_ref_;
        std::atomic<float>& swap_ref_;
        std::atomic<float>& analyzer_mag_type_ref_;
        std::atomic<float>& analyzer_min_db_ref_;
        std::atomic<float>& analyzer_time_length_ref_;
        std::atomic<float>& analyzer_move_type_ref_;

        AtomicBound<float> atomic_bound_;
        uint64_t generation_{};
        zldsp::analyzer::FIFOTransferBuffer<2> transfer_buffer_;
        size_t consumer_id_{};

        float db1_{-240.f}, db2_{-240.f};
        zldsp::vector::aligned_vector<float> xs_{}, y1s_{}, y2s_{};
        zlchore::thread::TriBuffer<juce::Path> path1_, path2_;

        double start_time_{0.0};
        double motion_start_time_{0.0};
        double previous_time_stamp_{kUnsetHistoryTimeStamp};
        float db_to_y_scale_{0.f};
        float db_to_y_bias_{0.f};
        bool is_y_mapping_initialized_{false};
        float curve_thickness_{0.f};

        enum class MoveType {
            kSync, kSlow, kRoll
        };

        MoveType move_type_{MoveType::kSync};
        zldsp::analyzer::MagType mag_type_{zldsp::analyzer::MagType::kPeak};
        size_t roll_next_point_{0};

        bool is_first_point_{true};
        bool reset_requested_{false};
        int too_much_samples_{0};
        int num_missing_points_{0};

        double sample_rate_{0.};
        size_t max_num_samples_{0};
        float time_length_idx_{0.f}, time_length_{6.f};

        size_t num_points_{0};
        int num_samples_per_point_{0};
        int num_points_per_second_{0};
        double second_per_point_{0};

        [[nodiscard]] bool shouldResetHistory(double next_time_stamp) const;

        void resetHistory(double time_stamp);

        void updateYMapping(juce::Rectangle<float> bound, float min_db);

        void updateXs(juce::Rectangle<float> bound, double next_time_stamp);

        void updatePaths();

        void lookAndFeelChanged() override;
    };
}
