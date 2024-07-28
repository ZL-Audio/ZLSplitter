// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef LR_SPLITER_HPP
#define LR_SPLITER_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input left signal and right signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class LRSplitter {
    public:
        LRSplitter() = default;

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal left buffer and right buffer
         * @param buffer
         */
        void split(juce::AudioBuffer<FloatType> &buffer);

        /**
         * combine the internal left buffer and right buffer into the audio buffer
         * @param buffer
         */
        void combine(juce::AudioBuffer<FloatType> &buffer);

        inline juce::AudioBuffer<FloatType> &getLBuffer() { return lBuffer; }

        inline juce::AudioBuffer<FloatType> &getRBuffer() { return rBuffer; }

    private:
        juce::AudioBuffer<FloatType> lBuffer, rBuffer;
    };
}

#endif //LR_SPLITER_HPP
