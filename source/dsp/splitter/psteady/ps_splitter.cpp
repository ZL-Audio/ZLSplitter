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
        peakBuffer.set_capacity(static_cast<size_t>(spec.sampleRate * .001));
        peakBuffer.clear();
        steadyBuffer.set_capacity(static_cast<size_t>(spec.sampleRate * 1.));
        steadyBuffer.clear();
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
            while (peakBuffer.size() >= peakBufferSize) {
                peakSM = peakSM - peakBuffer.pop_front();
            }
            while (steadyBuffer.size() >= steadyBufferSize) {
                steadySM = steadySM - steadyBuffer.pop_front();
            }
            const auto square = data[i] * data[i];
            peakBuffer.push_back(square);
            steadyBuffer.push_back(square);
            peakSM += square;
            steadySM += square;
            const auto currentThreshold = steadySM * steadyBufferSizeInverse * currentBalance;
            if (peakSM * peakBufferSizeInverse > currentThreshold) {
                mask = mask * currentAttack + currentAttackC;
            } else {
                mask *= currentRelease;
            }
            const auto peak = data[i] * mask;
            pData[i] = peak;
            sData[i] = data[i] - peak;
        }
    }

    template<typename FloatType>
    void PSSplitter<FloatType>::updatePara() {
        currentBalance = std::pow(FloatType(10), FloatType(1) - balance.load());
        currentBalance = currentBalance * currentBalance;
        currentRelease = std::pow(FloatType(0.9) * cube(hold.load()) + FloatType(5e-2), FloatType(10) / sampleRate.load());
        currentAttack = std::pow(FloatType(1e-4), (FloatType(500) - FloatType(450) * attack.load()) / sampleRate.load());
        currentAttackC = FloatType(1) - currentAttack;
        const auto currentSmooth = std::max(smooth.load(), FloatType(0.01));
        peakBufferSize = peakBuffer.capacity();
        peakBufferSizeInverse = FloatType(1) / static_cast<FloatType>(peakBufferSize);
        steadyBufferSize = static_cast<size_t>(currentSmooth * static_cast<FloatType>(steadyBuffer.capacity()));
        steadyBufferSize = std::max(steadyBufferSize, peakBufferSize);
        steadyBufferSizeInverse = FloatType(1) / static_cast<FloatType>(steadyBufferSize);
    }

    template
    class PSSplitter<float>;

    template
    class PSSplitter<double>;
} // zlSplitter
