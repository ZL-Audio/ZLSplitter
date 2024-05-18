// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "splitter/splitter.hpp"
#include "dsp_definitions.hpp"

namespace zlDSP {
    class Controller {
    public:
        Controller();

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<double> &buffer);

        void setType(splitType::stype x) {
            splitType.store(x);
        }

        void setMix(double x) {
            mix.store(x);
        }

        zlSplitter::LRSplitter<double> &getLRSplitter() {
            return lrSplitter;
        }

        zlSplitter::MSSplitter<double> &getMSSplitter() {
            return msSplitter;
        }

        zlSplitter::LHSplitter<double> &getLHSplitter() {
            return lhSplitter;
        }

        void setSwap(const bool x) { swap.store(x); }

    private:
        std::atomic<splitType::stype> splitType;
        zlSplitter::LRSplitter<double> lrSplitter;
        zlSplitter::MSSplitter<double> msSplitter;
        zlSplitter::LHSplitter<double> lhSplitter;
        zlSplitter::TSSplitter<double> tsSplitter;
        std::atomic<double> mix{0.0};
        std::atomic<bool> swap{false};

        void processLR(juce::AudioBuffer<double> &buffer);

        void processMS(juce::AudioBuffer<double> &buffer);

        void processLH(juce::AudioBuffer<double> &buffer);
    };
} // zlDSP

#endif //CONTROLLER_HPP
