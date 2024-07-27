// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef LH_LINEAR_SPLITTER_HPP
#define LH_LINEAR_SPLITTER_HPP

#include "../../reverse_iir_filter/reverse_iir_filter.hpp"
#include "../../iir_filter/iir_base.hpp"

namespace zlSplitter {
    /**
    * a linear-phase splitter that splits the stereo audio signal input low-freq signal and high-freq signal
    * @tparam FloatType
    */
    template<typename FloatType>
    class LHLinearSplitter {
        LHLinearSplitter();

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal low-freq buffer and high-freq buffer
         * @param buffer
         */
        void split(juce::AudioBuffer<FloatType> &buffer);

        /**
         * split the audio block into internal low-freq buffer and high-freq buffer
         * @param block
         */
        void split(juce::dsp::AudioBlock<FloatType> block);

        void setFreq(FloatType x);

        void setOrder(size_t x);

        inline juce::AudioBuffer<FloatType> &getLBuffer() { return lBuffer; }

        inline juce::AudioBuffer<FloatType> &getHBuffer() { return hBuffer; }

        inline int getLatency() const {
            switch (order.load()) {
                case 1:
                case 2: return 2047;
                case 4: return 2047 * 2;
                default: return 0;
            }
        }

    private:
        juce::AudioBuffer<FloatType> lBuffer, hBuffer;
        juce::dsp::DelayLine<FloatType> delay;

        zlReverseIIR::ReverseIIRBase<FloatType> reverseFilter;
        zlIIR::IIRBase<FloatType> forwardFilter;

        std::atomic<FloatType> freq{FloatType(1000)};
        FloatType currentFreq{FloatType(1000)};
        std::atomic<size_t> order{2};
        size_t currentOrder{2};
    };
} // zlSplitter

#endif //LH_LINEAR_SPLITTER_HPP
