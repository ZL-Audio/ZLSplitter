// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef PS_SPLITTER_HPP
#define PS_SPLITTER_HPP

#include <numbers>
#include <juce_dsp/juce_dsp.h>

#include "../../container/container.hpp"

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input peak signal and steady signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class PSSplitter {
    public:
        PSSplitter() = default;

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal peak buffer and steady buffer
         * @param buffer
         */
        void split(juce::AudioBuffer<FloatType> &buffer);

        void split(juce::dsp::AudioBlock<FloatType> block);

        inline juce::AudioBuffer<FloatType> &getPBuffer() { return pBuffer; }

        inline juce::AudioBuffer<FloatType> &getSBuffer() { return sBuffer; }

        inline void setBalance(const FloatType x) {
            balance.store(x);
            toUpdate.store(true);
        }

        inline void setAttack(const FloatType x) {
            attack.store(x);
            toUpdate.store(true);
        }

        inline void setHold(const FloatType x) {
            hold.store(x);
            toUpdate.store(true);
        }

        inline void setSmooth(const FloatType x) {
            smooth.store(x);
            toUpdate.store(true);
        }

    private:
        juce::AudioBuffer<FloatType> pBuffer, sBuffer;
        std::atomic<FloatType> attack{FloatType(0.5)}, balance{FloatType(0.5)}, hold{FloatType(0.5)}, smooth{FloatType(0.5)};
        std::atomic<FloatType> sampleRate{FloatType(48000)};
        std::atomic<bool> toUpdate{true};
        FloatType currentBalance{}, currentRelease{}, currentAttack{}, currentAttackC{};
        FloatType peakBufferSizeInverse{1}, steadyBufferSizeInverse{1};
        size_t peakBufferSize{1}, steadyBufferSize{1};
        FloatType peakSM{0}, steadySM{0}, mask{0};
        zlContainer::CircularBuffer<FloatType> peakBuffer{1}, steadyBuffer{1};

        void updatePara();

        inline static constexpr FloatType cube(const FloatType x) {
            return x * x * x;
        }
    };
} // zlSplitter

#endif //PS_SPLITTER_HPP
