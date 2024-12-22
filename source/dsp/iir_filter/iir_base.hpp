// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef IIR_BASE_HPP
#define IIR_BASE_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlIIR {
    template<typename SampleType>
    class IIRBase {
    public:
        IIRBase() = default;

        void prepare(const juce::dsp::ProcessSpec &spec) {
            s1.resize(spec.numChannels);
            s2.resize(spec.numChannels);
            reset();
        }

        void reset() {
            std::fill(s1.begin(), s1.end(), static_cast<SampleType>(0));
            std::fill(s2.begin(), s2.end(), static_cast<SampleType>(0));
        }

        void snapToZero() {
            for (auto v: {&s1, &s2}) {
                for (auto &element: *v) {
                    juce::dsp::util::snapToZero(element);
                }
            }
        }

        template<typename ProcessContext>
        void process(const ProcessContext &context) noexcept {
            const auto &inputBlock = context.getInputBlock();
            auto &outputBlock = context.getOutputBlock();
            const auto numChannels = outputBlock.getNumChannels();
            const auto numSamples = outputBlock.getNumSamples();

            jassert(inputBlock.getNumChannels() <= s1.size());

            if (context.isBypassed) {
                if (context.usesSeparateInputAndOutputBlocks()) {
                    outputBlock.copyFrom(inputBlock);
                }
                for (size_t channel = 0; channel < numChannels; ++channel) {
                    auto *inputSamples = inputBlock.getChannelPointer(channel);
                    for (size_t i = 0; i < numSamples; ++i) {
                        processSample(channel, inputSamples[i]);
                    }
                }
            } else {
                for (size_t channel = 0; channel < numChannels; ++channel) {
                    auto *inputSamples = inputBlock.getChannelPointer(channel);
                    auto *outputSamples = outputBlock.getChannelPointer(channel);
                    for (size_t i = 0; i < numSamples; ++i) {
                        outputSamples[i] = processSample(channel, inputSamples[i]);
                    }
                }
            }

#if JUCE_DSP_ENABLE_SNAP_TO_ZERO
            snapToZero();
#endif
        }

        SampleType processSample(const size_t channel, SampleType inputValue) {
            const auto outputValue = inputValue * coeff[0] + s1[channel];
            s1[channel] = (inputValue * coeff[1]) - (outputValue * coeff[3]) + s2[channel];
            s2[channel] = (inputValue * coeff[2]) - (outputValue * coeff[4]);
            return outputValue;
        }

        void updateFromBiquad(const std::array<SampleType, 3> a, const std::array<SampleType, 3> b) {
            const auto a0Inv = 1.0 / a[0];
            coeff[0] = static_cast<SampleType>(b[0] * a0Inv);
            coeff[1] = static_cast<SampleType>(b[1] * a0Inv);
            coeff[2] = static_cast<SampleType>(b[2] * a0Inv);
            coeff[3] = static_cast<SampleType>(a[1] * a0Inv);
            coeff[4] = static_cast<SampleType>(a[2] * a0Inv);
        }

    private:
        std::array<SampleType, 5> coeff{0, 0, 0, 0, 0};
        SampleType g, R2, h, chp, cbp, clp;
        std::vector<SampleType> s1, s2;
    };
}

#endif //IIR_BASE_HPP
