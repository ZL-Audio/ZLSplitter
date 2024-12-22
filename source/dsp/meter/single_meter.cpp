// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "single_meter.hpp"

namespace zlMeter {
    template<typename FloatType>
    void SingleMeter<FloatType>::reset() {
        for (size_t idx = 0; idx < maxPeak.size(); ++idx) {
            maxPeak[idx].store(-160.f);
            bufferPeak[idx].store(-160.f);
        }
    }

    template<typename FloatType>
    void SingleMeter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(static_cast<FloatType>(spec.sampleRate));
        reset();
    }

    template<typename FloatType>
    void SingleMeter<FloatType>::process(juce::dsp::AudioBlock<FloatType> block) {
        if (!isON.load()) return;
        const auto decay = decayRate.load() * static_cast<FloatType>(block.getNumSamples()) / sampleRate.load();

        std::fill(tempPeak.begin(), tempPeak.end(), FloatType(0));
        for (size_t channel = 0; channel < maxPeak.size(); ++channel) {
            for (size_t idx = 0; idx < block.getNumSamples(); ++idx) {
                tempPeak[channel] = std::max(tempPeak[channel],
                                             std::abs(block.getSample(
                                                 static_cast<int>(channel), static_cast<int>(idx))));
            }
        }

        for (size_t channel = 0; channel < maxPeak.size(); ++channel) {
            tempPeak[channel] = juce::Decibels::gainToDecibels(tempPeak[channel]);
            const auto currentPeak = bufferPeak[channel].load() - currentDecay[channel];
            if (currentPeak <= tempPeak[channel]) {
                bufferPeak[channel].store(tempPeak[channel]);
                currentDecay[channel] = 0;
            } else {
                bufferPeak[channel].store(currentPeak);
                currentDecay[channel] += decay;
            }
            maxPeak[channel].store(std::max(maxPeak[channel].load(), tempPeak[channel]));
        }
    }

    template
    class SingleMeter<float>;

    template
    class SingleMeter<double>;
} // zlMeter
