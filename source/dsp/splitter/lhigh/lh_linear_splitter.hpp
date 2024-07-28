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
    public:
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

        void setFreq(const FloatType x) {
            freq.store(x);
            toUpdate.store(true);
        }

        void setOrder(const size_t x) {
            order.store(x);
            toUpdate.store(true);
        }

        inline juce::AudioBuffer<FloatType> &getLBuffer() { return lBuffer; }

        inline juce::AudioBuffer<FloatType> &getHBuffer() { return hBuffer; }

        inline int getLatency() const {
            switch (order.load()) {
                case 1: return static_cast<int>(1 << (firstOrderNumStage + extraStage.load() + 1));
                case 2: return static_cast<int>(1 << (secondOrderNumStage + extraStage.load() + 1)) + 1;
                case 4: return static_cast<int>(1 << (secondOrderNumStage + extraStage.load() + 2)) + 2;
                default: return 0;
            }
        }

    private:
        inline static constexpr size_t firstOrderNumStage = 9;
        inline static constexpr size_t secondOrderNumStage = 11;
        std::atomic<size_t> extraStage{0};

        juce::AudioBuffer<FloatType> lBuffer, hBuffer;
        juce::dsp::DelayLine<FloatType, juce::dsp::DelayLineInterpolationTypes::None> delay;

        zlReverseIIR::ReverseFirstOrderIIRBase<FloatType> reverseFirstOrderFilter{firstOrderNumStage};

        std::array<zlReverseIIR::ReverseIIRBase<FloatType>, 2> reverseFilter = {
            zlReverseIIR::ReverseIIRBase<FloatType>(secondOrderNumStage),
            zlReverseIIR::ReverseIIRBase<FloatType>(secondOrderNumStage)
        };
        std::array<zlIIR::IIRBase<FloatType>, 2> forwardFilter;

        std::atomic<double> sampleRate;
        std::atomic<FloatType> freq{FloatType(1000)};
        FloatType currentFreq{FloatType(1000)};
        std::atomic<size_t> order{0};
        size_t currentOrder{0};

        std::atomic<bool> toUpdate{true}, toReset{false};

        void update();

        static constexpr double order2q = 0.70710678118654757274; // np.sqrt(2) / 2
        static constexpr double order4q1 = 0.54119610014619701222; // 1 / (2 * np.cos(np.pi / 8))
        static constexpr double order4q2 = 1.30656296487637635373; // 1 / (2 * np.cos(np.pi / 8 * 3))
    };
} // zlSplitter

#endif //LH_LINEAR_SPLITTER_HPP
