// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_SINGLE_METER_HPP
#define ZL_SINGLE_METER_HPP

// #include "juce_audio_processors/juce_audio_processors.h"
#include "juce_dsp/juce_dsp.h"

namespace zlMeter {
    template<typename FloatType>
    class SingleMeter {
    public:
        SingleMeter() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        void process(juce::dsp::AudioBlock<FloatType> block);

        std::array<std::atomic<FloatType>, 2> &getmaxPeak() { return maxPeak; }

        std::array<std::atomic<FloatType>, 2> &getBufferPeak() { return bufferPeak; }

        void enable(const bool x) { isON.store(x); }

    private:
        std::array<std::atomic<FloatType>, 2> maxPeak, bufferPeak;
        std::array<FloatType, 2> tempPeak;
        std::atomic<FloatType> decayRate{2}, sampleRate;
        std::array<FloatType, 2> currentDecay;
        std::atomic<bool> isON = false;
    };
} // zlMeter

#endif //ZL_SINGLE_METER_HPP
