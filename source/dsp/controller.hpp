// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "splitter/splitter.hpp"
#include "meter/meter.hpp"
#include "dsp_definitions.hpp"

namespace zlDSP {
    class Controller {
    public:
        Controller();

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::AudioBuffer<double> &buffer);

        void setType(const splitType::stype x) {
            mSplitType.store(x);
        }

        void setMix(const double x) {
            mix.store(x);
        }

        void setLHFilterType(const lhFilterType::ftype x) {
            lhFilterType.store(x);
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

        zlSplitter::LHLinearSplitter<double> &getLHLinearSplitter() {
            return lhLinearSplitter;
        }

        zlSplitter::TSSplitter<double> &getTSSplitter(const size_t i) {
            return tsSplitters[i];
        }

        void setSwap(const bool x) { swap.store(x); }

        zlMeter::SingleMeter<double> &getMeter1() { return meter1; }

        zlMeter::SingleMeter<double> &getMeter2() { return meter2; }

        inline int getLatency() const {
            switch (mSplitType.load()) {
                case splitType::lright:
                case splitType::mside: {
                    return 0;
                }
                case splitType::lhigh: {
                    switch (lhFilterType.load()) {
                        case lhFilterType::svf:
                            return 0;
                        case lhFilterType::fir:
                            return lhLinearSplitter.getLatency();
                    }
                }
                case splitType::tsteady: {
                    return tsSplitters[0].getLatency();
                }
                case splitType::psteady: {
                    return 0;
                }
                case splitType::numSplit: {
                    return 0;
                }
            }
        }

    private:
        std::atomic<splitType::stype> mSplitType;
        std::atomic<lhFilterType::ftype> lhFilterType;
        zlSplitter::LRSplitter<double> lrSplitter;
        zlSplitter::MSSplitter<double> msSplitter;
        zlSplitter::LHSplitter<double> lhSplitter;
        zlSplitter::LHLinearSplitter<double> lhLinearSplitter;
        std::array<zlSplitter::TSSplitter<double>, 2> tsSplitters;
        std::atomic<double> mix{0.0};
        double currentMix{0.0};
        juce::SmoothedValue<double> currentMixSmooth{0.0};
        std::atomic<bool> swap{false};

        zlMeter::SingleMeter<double> meter1, meter2;

        juce::AudioBuffer<double> internalBuffer;

        void processLR(juce::AudioBuffer<double> &buffer);

        void processMS(juce::AudioBuffer<double> &buffer);

        void processLH(juce::AudioBuffer<double> &buffer);

        void processTS(juce::AudioBuffer<double> &buffer);
    };
} // zlDSP

#endif //CONTROLLER_HPP
