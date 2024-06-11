// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

// Fitzgerald, D. (2010). Harmonic/percussive separation using median filtering.

#ifndef TS_SPLITTER_HPP
#define TS_SPLITTER_HPP

#include <numbers>
#include <juce_dsp/juce_dsp.h>

#include "../../median_filter/median_filter.hpp"

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input transient signal and steady signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class TSSplitter {
    public:
        TSSplitter() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal transient buffer and steady buffer
         * @param buffer
         */
        void split(juce::AudioBuffer<FloatType> &buffer);

        /**
         * split the audio block into internal transient buffer and steady buffer
         * @param block
         */
        void split(juce::dsp::AudioBlock<FloatType> block);

    private:
        std::vector<zlMedianFilter::HeapFilter<FloatType, 17>> timeMedian{};
        zlMedianFilter::HeapFilter<FloatType, 17> freqMedian{};
    };
} // zlSplitter

#endif //TS_SPLITTER_HPP
