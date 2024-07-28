// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "lh_linear_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    LHLinearSplitter<FloatType>::LHLinearSplitter() = default;

    template<typename FloatType>
    void LHLinearSplitter<FloatType>::reset() {
        delay.reset();
    }

    template<typename FloatType>
    void LHLinearSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        sampleRate.store(spec.sampleRate);
        reverseFirstOrderFilter.prepare(spec);
        reverseFilter[0].prepare(spec);
        reverseFilter[1].prepare(spec);
        forwardFilter[0].prepare(spec);
        forwardFilter[1].prepare(spec);
        delay.setMaximumDelayInSamples(static_cast<int>(1 << (secondOrderNumStage + 2)) + 2);
        delay.prepare(spec);
        toUpdate.store(true);
    }

    template<typename FloatType>
    void LHLinearSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        split(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void LHLinearSplitter<FloatType>::split(juce::dsp::AudioBlock<FloatType> block) {
        if (toUpdate.exchange(false)) {
            update();
        }
        auto subBlock = block.getSubsetChannelBlock(0, 2);

        lBuffer.setSize(
            static_cast<int>(subBlock.getNumChannels()),
            static_cast<int>(subBlock.getNumSamples()),
            false, false, true);
        hBuffer.setSize(
            static_cast<int>(subBlock.getNumChannels()),
            static_cast<int>(subBlock.getNumSamples()),
            false, false, true);
        juce::dsp::AudioBlock<FloatType> lBlock{lBuffer};
        juce::dsp::AudioBlock<FloatType> hBlock{hBuffer};
        lBlock.copyFrom(subBlock);
        hBlock.copyFrom(subBlock);
        juce::dsp::ProcessContextReplacing<FloatType> lContext{lBlock};
        juce::dsp::ProcessContextReplacing<FloatType> hContext{hBlock};

        switch (currentOrder) {
            case 1: {
                reverseFirstOrderFilter.process(lBlock);
                forwardFilter[0].process(lContext);
                break;
            }
            case 2: {
                reverseFilter[0].process(lBlock);
                forwardFilter[0].process(lContext);
                break;
            }
            case 4: {
                reverseFilter[0].process(lBlock);
                forwardFilter[0].process(lContext);
                reverseFilter[1].process(lBlock);
                forwardFilter[1].process(lContext);
                break;
            }
            default: {}
        }

        delay.process(hContext);
        hBlock.subtract(lBlock);
    }

    template<typename FloatType>
    void LHLinearSplitter<FloatType>::update() {
        if (currentOrder != order.load()) {
            currentOrder = order.load();
            delay.setDelay(getLatency());
            switch (currentOrder) {
                case 1: {
                    reverseFirstOrderFilter.reset();
                    forwardFilter[0].reset();
                    break;
                }
                case 2: {
                    reverseFilter[0].reset();
                    forwardFilter[0].reset();
                    break;
                }
                case 4: {
                    reverseFilter[0].reset();
                    forwardFilter[0].reset();
                    reverseFilter[1].reset();
                    forwardFilter[1].reset();
                    break;
                }
                default: {
                }
            }
        }
        currentFreq = freq.load();

        switch (currentOrder) {
            case 1: {
                const auto coeff = juce::dsp::IIR::ArrayCoefficients<FloatType>::makeFirstOrderLowPass(
                    sampleRate.load(), currentFreq);
                reverseFirstOrderFilter.updateFromBiquad({coeff[2], coeff[3]}, {coeff[0], coeff[1]});
                forwardFilter[0].updateFromBiquad({coeff[2], coeff[3], 0}, {coeff[0], coeff[1], 0});
                break;
            }
            case 2: {
                const auto coeff = juce::dsp::IIR::ArrayCoefficients<FloatType>::makeLowPass(
                    sampleRate.load(), currentFreq, static_cast<FloatType>(order2q));
                reverseFilter[0].updateFromBiquad({coeff[3], coeff[4], coeff[5]}, {coeff[0], coeff[1], coeff[2]});
                forwardFilter[0].updateFromBiquad({coeff[3], coeff[4], coeff[5]}, {coeff[0], coeff[1], coeff[2]});
                break;
            }
            case 4: {
                {
                    const auto coeff = juce::dsp::IIR::ArrayCoefficients<FloatType>::makeLowPass(
                        sampleRate.load(), currentFreq, static_cast<FloatType>(order4q1));
                    reverseFilter[0].updateFromBiquad({coeff[3], coeff[4], coeff[5]}, {coeff[0], coeff[1], coeff[2]});
                    forwardFilter[0].updateFromBiquad({coeff[3], coeff[4], coeff[5]}, {coeff[0], coeff[1], coeff[2]});
                } {
                    const auto coeff = juce::dsp::IIR::ArrayCoefficients<FloatType>::makeLowPass(
                        sampleRate.load(), currentFreq, static_cast<FloatType>(order4q2));
                    reverseFilter[1].updateFromBiquad({coeff[3], coeff[4], coeff[5]}, {coeff[0], coeff[1], coeff[2]});
                    forwardFilter[1].updateFromBiquad({coeff[3], coeff[4], coeff[5]}, {coeff[0], coeff[1], coeff[2]});
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    template
    class LHLinearSplitter<float>;

    template
    class LHLinearSplitter<double>;
} // zlSplitter
