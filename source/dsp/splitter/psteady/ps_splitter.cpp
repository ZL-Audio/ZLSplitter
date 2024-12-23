// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ps_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    void PSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        pBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        sBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        sampleRate.store(static_cast<FloatType>(spec.sampleRate));
        circularBuffer.set_capacity(static_cast<size_t>(spec.sampleRate * 1.));
        toUpdate.store(true);
    }

    template<typename FloatType>
    void PSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        split(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void PSSplitter<FloatType>::split(juce::dsp::AudioBlock<FloatType> block) {
        if (toUpdate.exchange(false)) {
            updatePara();
        }
        pBuffer.setSize(1, static_cast<int>(block.getNumSamples()), false, false, true);
        sBuffer.setSize(1, static_cast<int>(block.getNumSamples()), false, false, true);
        const auto *data = block.getChannelPointer(0);
        auto *pData = pBuffer.getWritePointer(0);
        auto *sData = sBuffer.getWritePointer(0);
        for (size_t i = 0; i < block.getNumSamples(); ++i) {
            while (circularBuffer.size() >= currentBufferSize) {
                squareSum = squareSum - circularBuffer.pop_front();
            }
            const auto square = data[i] * data[i];
            circularBuffer.push_back(square);
            squareSum += square;
            const auto currentThreshold = std::sqrt(squareSum * currentBufferSizeInverse) * currentBalance;
            if (std::abs(data[i]) > currentThreshold) {
                mask = mask * currentAttack + currentAttackC;
            } else {
                mask *= currentRelease;
            }
            const auto peak = data[i] * mask;
            const auto steady = data[i] - peak;
            pData[i] = steady * currentMix + peak * currentMixC;
            sData[i] = data[i] - pData[i];
        }
    }

    template<typename FloatType>
    void PSSplitter<FloatType>::updatePara() {
        currentMix = mix.load();
        currentMixC = FloatType(1) - currentMix;
        currentBalance = std::pow(FloatType(10), FloatType(1) - balance.load());
        currentRelease = std::pow(FloatType(0.9) * hold.load() + FloatType(0.01), FloatType(10) / sampleRate.load());
        currentAttack = std::pow(FloatType(1e-5), FloatType(50) / sampleRate.load());
        currentAttackC = FloatType(1) - currentAttack;
        currentBufferSize = std::max(static_cast<size_t>(1),
                                     static_cast<size_t>(
                                         smooth.load() * static_cast<FloatType>(circularBuffer.capacity())));
        currentBufferSizeInverse = FloatType(1) / static_cast<FloatType>(currentBufferSize);
    }

    template
    class PSSplitter<float>;

    template
    class PSSplitter<double>;
} // zlSplitter
