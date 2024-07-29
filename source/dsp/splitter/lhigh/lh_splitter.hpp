// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef LH_SPLITTER_HPP
#define LH_SPLITTER_HPP

#include <numbers>
#include <juce_dsp/juce_dsp.h>
#include "../../reverse_iir_filter/reverse_iir_filter.hpp"

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input low-freq signal and high-freq signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class LHSplitter {
    public:
        LHSplitter();

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

    private:
        juce::AudioBuffer<FloatType> lBuffer, hBuffer;
        std::array<juce::dsp::FirstOrderTPTFilter<FloatType>, 2> low1, high1;
        std::array<juce::dsp::StateVariableTPTFilter<FloatType>, 4> low2, high2;

        size_t fitlerNum{0};
        std::atomic<FloatType> freq{FloatType(1000)};
        FloatType currentFreq{FloatType(1000)};
        std::atomic<size_t> order{0};
        size_t currentOrder{0};
        std::atomic<bool> toUpdate{true}, toUpdateOrder{true}, toReset{false};

        void update();

        void updateOrder();

        static constexpr double order2q = 0.7071067811865476; // np.sqrt(2) / 2
        static constexpr double order4q1 = 0.541196100146197; // 1 / (2 * np.cos(np.pi / 8))
        static constexpr double order4q2 = 1.3065629648763764; // 1 / (2 * np.cos(np.pi / 8 * 3))
    };
} // zlSplitter

#endif //LH_SPLITTER_HPP
