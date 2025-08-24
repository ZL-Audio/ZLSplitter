// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

//
// Created by Zishu Liu on 8/23/25.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class MagAnalyzerPanel final : public juce::Component,
                                   private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit MagAnalyzerPanel(PluginProcessor &p, zlgui::UIBase &base);

        ~MagAnalyzerPanel() override;

        void paint(juce::Graphics &g) override;

        void run(double next_time_stamp);

        void resized() override;

    private:
        PluginProcessor &p_ref_;
        zlgui::UIBase &base_;
        std::atomic<float> &split_type_ref_, &swap_ref_, &fft_min_db_ref_;

        static constexpr std::array kNAIDs{
            zlstate::PMagType::kID,
            zlstate::PMagMinDB::kID,
            zlstate::PMagTimeLength::kID
        };

        zldsp::analyzer::MultipleMagAnalyzer<double, 2, zlp::Controller<double>::kAnalyzerPointNum> &mag_analyzer_ref_;
        AtomicBound<float> atomic_bound_;

        std::array<float, zlp::Controller<double>::kAnalyzerPointNum> xs_{}, out1_ys_{}, out2_ys_{};
        juce::Path out_path1_, next_out_path1_;
        juce::Path out_path2_, next_out_path2_;
        std::mutex mutex_;

        std::atomic<bool> to_reset_path_{true};
        double start_time_{0.0}, current_time{0.0};
        double current_count_{0.0};
        std::atomic<double> num_per_second_{50.0};

        bool is_first_point_{true};

        std::atomic<float> analyzer_min_db_{-72.f};

        void updatePaths(juce::Rectangle<float> bound);

        void parameterChanged(const juce::String &parameter_id, float new_value) override;

        void setTimeLength(const float x) {
            mag_analyzer_ref_.setTimeLength(x);
            num_per_second_.store(
                static_cast<double>(zlp::Controller<double>::kAnalyzerPointNum - 1) / static_cast<double>(x));
            to_reset_path_.exchange(true, std::memory_order::release);
        }
    };
}
