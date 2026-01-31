// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../helper/helper.hpp"
#include "../../../gui/gui.hpp"

namespace zlpanel {
    class FFTBackgroundPanel final : public juce::Component {
    public:
        explicit FFTBackgroundPanel(PluginProcessor &processor, zlgui::UIBase &base);

        void resized() override;

        void repaintCallBackSlow();

        void setMinMaxFreq(double min_freq, double max_freq);

    private:
        static constexpr std::array<double, 11> kBackgroundFreqs = {
            20.0, 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0, 10000.0, 20000.0, 50000.0
        };

        static constexpr std::array<std::string_view, 11> kBackgroundFreqsNames = {
            "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k", "50k"
        };

        class Background1 final : public juce::Component {
        public:
            explicit Background1(zlgui::UIBase &base);

            void paint(juce::Graphics &g) override;

            void setMinFreq(const double x) {
                min_freq_ = x;
            }

            void setMaxFreq(const double x) {
                max_freq_ = x;
            }

            void setMinDB(const float x) {
                min_db_ = x;
            }

        private:
            zlgui::UIBase &base_;

            double min_freq_{10.0}, max_freq_{22000.0};
            float min_db_{-72.f};
        };

        zlgui::UIBase &base_;
        Background1 background1_;

        zlgui::attachment::ComponentUpdater updater_;

        zlgui::combobox::CompactCombobox fft_min_freq_box_;
        zlgui::attachment::ComboBoxAttachment<true> fft_min_freq_attach_;

        zlgui::combobox::CompactCombobox fft_max_freq_box_;
        zlgui::attachment::ComboBoxAttachment<true> fft_max_freq_attach_;

        zlgui::combobox::CompactCombobox fft_min_db_box_;
        zlgui::attachment::ComboBoxAttachment<true> fft_min_db_attach_;
        int c_min_db_index_{-1};
    };
} // zlpanel