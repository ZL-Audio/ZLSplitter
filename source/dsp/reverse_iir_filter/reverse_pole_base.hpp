// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#ifndef REVERSE_POLE_BASE_HPP
#define REVERSE_POLE_BASE_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlReverseIIR {
    template<typename SampleType>
    class ReversePoleBase {

        explicit ReversePoleBase(const size_t stage) : numStage(stage) {
            as.resize(stage + 1);
            bs.resize(stage + 1);
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            us.resize(static_cast<size_t>(spec.numChannels));
            vs.resize(static_cast<size_t>(spec.numChannels));
            uDelays.resize(static_cast<size_t>(spec.numChannels));
            vDelays.resize(static_cast<size_t>(spec.numChannels));

            for (auto &buffer : us) {
                buffer.resize(static_cast<size_t>(spec.maximumBlockSize));
            }

            for (auto &buffer : vs) {
                buffer.resize(static_cast<size_t>(spec.maximumBlockSize));
            }

            for (auto &delay : uDelays) {
                int numDelay = 1;
                for (size_t i = 0; i < numStage + 1; ++i) {
                    delay[i].setMaximumDelayInSamples(numDelay);
                    delay[i].setDelay(static_cast<SampleType>(numDelay));
                    numDelay *= 2;
                }
            }

            for (auto &delay : vDelays) {
                int numDelay = 1;
                for (size_t i = 0; i < numStage + 1; ++i) {
                    delay[i].setMaximumDelayInSamples(numDelay);
                    delay[i].setDelay(static_cast<SampleType>(numDelay));
                    numDelay *= 2;
                }
            }
        }

        void process(juce::AudioBuffer<SampleType> &buffer) {
            process(juce::dsp::AudioBlock<SampleType>(buffer));
        }

        void process(juce::dsp::AudioBlock<SampleType> block) {
            for (size_t channel = 0; channel < static_cast<size_t>(block.getNumChannels()); ++channel) {
                std::vector<SampleType> &u(us[channel]), &v(vs[channel]);

                if (u.size() < static_cast<size_t>(block.getNumSamples())) {
                    u.resize(static_cast<size_t>(block.getNumSamples()));
                    v.resize(static_cast<size_t>(block.getNumSamples()));
                }

                std::vector<juce::dsp::DelayLine<SampleType>> &uDelay(uDelays[channel]);
                std::vector<juce::dsp::DelayLine<SampleType>> &vDelay(vDelays[channel]);

                auto currentBlock = block.getSingleChannelBlock(channel);
                for (size_t index = 0; index < static_cast<size_t>(block.getNumSamples()); ++index) {
                    {
                        const auto current = currentBlock.getSample(0, static_cast<int>(index));
                        u[index] = as[0] * current + uDelay[0].popSample(0);
                        v[index] = bs[0] * current;
                        uDelay[0].pushSample(0, current);
                    }
                    for (size_t stage = 1; stage <= numStage; ++stage) {
                        const auto uCurrent = u[index];
                        const auto vCurrent = v[index];
                        u[index] = as[stage] * uCurrent - bs[stage] * vCurrent + uDelay[stage].popSample(0);
                        v[index] = bs[stage] * uCurrent + as[stage] * vCurrent + vDelay[stage].popSample(0);

                        uDelay[stage].pushSample(0, uCurrent);
                        vDelay[stage].pushSample(0, vCurrent);
                    }
                    currentBlock.setSample(0, static_cast<int>(index), u[index] + aB * v[index]);
                }
            }
        }

        void updateFromBiquad(const std::array<double, 3> &a) {
            as[0] = -static_cast<SampleType>(a[1] / 2);
            bs[0] = -static_cast<SampleType>(std::sqrt(a[2] - a[1] * a[1] / 4));
            aB = as[0] / bs[0];
            for (size_t i = 1; i < numStage; ++i) {
                const auto ii = i - 1;
                as[i] = as[ii] * as[ii] - bs[ii] * bs[ii];
                bs[i] = 2 * as[ii] * bs[ii];
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(numStage))) - 1;
        }

    private:
        size_t numStage = 10;
        std::vector<std::vector<juce::dsp::DelayLine<SampleType>>> uDelays, vDelays;
        std::vector<SampleType> as, bs;
        SampleType aB;
        std::vector<std::vector<SampleType>> us, vs;
    };
}

#endif //REVERSE_POLE_BASE_HPP
