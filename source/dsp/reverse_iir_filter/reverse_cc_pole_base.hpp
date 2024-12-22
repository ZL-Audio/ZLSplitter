// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef REVERSE_CC_POLE_BASE_HPP
#define REVERSE_CC_POLE_BASE_HPP

#include <juce_dsp/juce_dsp.h>

#include "../delay/delay.hpp"

namespace zlReverseIIR {
    /**
     * reverse of a pair of complex conjugates poles
     * @tparam SampleType
     */
    template<typename SampleType>
    class ReverseCCPoleBase {
    public:
        explicit ReverseCCPoleBase(const size_t stage) {
            setNumStage(stage);
        }

        void setNumStage(const size_t stage) {
            numStage = stage;
            as.resize(stage + 1);
            bs.resize(stage + 1);
        }

        void reset() {
            for (auto &buffer: us) {
                std::fill(buffer.begin(), buffer.end(), SampleType(0));
            }

            for (auto &buffer: vs) {
                std::fill(buffer.begin(), buffer.end(), SampleType(0));
            }

            for (auto &delay: uDelays) {
                for (auto &d: delay) {
                    d.reset();
                }
            }

            for (auto &delay: vDelays) {
                for (auto &d: delay) {
                    d.reset();
                }
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            us.resize(static_cast<size_t>(spec.numChannels));
            vs.resize(static_cast<size_t>(spec.numChannels));
            uDelays.resize(static_cast<size_t>(spec.numChannels));
            vDelays.resize(static_cast<size_t>(spec.numChannels));

            for (auto &buffer: us) {
                buffer.resize(static_cast<size_t>(spec.maximumBlockSize));
            }

            for (auto &buffer: vs) {
                buffer.resize(static_cast<size_t>(spec.maximumBlockSize));
            }

            for (auto &delay: uDelays) {
                int numDelay = 1;
                delay.resize(numStage + 1);
                for (size_t i = 0; i < numStage + 1; ++i) {
                    delay[i].setMaximumDelayInSamples(numDelay);
                    delay[i].setDelay(numDelay);
                    numDelay *= 2;
                }
            }

            for (auto &delay: vDelays) {
                int numDelay = 1;
                delay.resize(numStage + 1);
                for (size_t i = 0; i < numStage + 1; ++i) {
                    delay[i].setMaximumDelayInSamples(numDelay);
                    delay[i].setDelay(numDelay);
                    numDelay *= 2;
                }
            }
            reset();
        }

        void process(juce::AudioBuffer<SampleType> &buffer) {
            process(juce::dsp::AudioBlock<SampleType>(buffer));
        }

        void process(juce::dsp::AudioBlock<SampleType> block) {
            if (isComplex) {
                processComplex(block);
            }
        }

        void updateFromBiquad(const std::array<double, 3> &a) {
            const auto delta = a[1] * a[1] / 4 - a[2];
            isComplex = (delta < 0);
            if (isComplex) {
                as[0] = -static_cast<SampleType>(a[1] / 2);
                bs[0] = -static_cast<SampleType>(std::sqrt(a[2] - a[1] * a[1] / 4));
                aB = as[0] / bs[0];
                for (size_t i = 1; i < numStage; ++i) {
                    const auto ii = i - 1;
                    as[i] = as[ii] * as[ii] - bs[ii] * bs[ii];
                    bs[i] = 2 * as[ii] * bs[ii];
                }
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(numStage))) + 1;
        }

    private:
        size_t numStage{0};
        std::vector<std::vector<zlDelay::FIFODelay<SampleType> > > uDelays, vDelays;
        std::vector<SampleType> as, bs;
        SampleType aB;
        std::vector<std::vector<SampleType> > us, vs;

        bool isComplex{false};

        void processComplex(juce::dsp::AudioBlock<SampleType> block) {
            for (size_t channel = 0; channel < static_cast<size_t>(block.getNumChannels()); ++channel) {
                std::vector<SampleType> &u(us[channel]), &v(vs[channel]);

                if (u.size() < static_cast<size_t>(block.getNumSamples())) {
                    u.resize(static_cast<size_t>(block.getNumSamples()));
                    v.resize(static_cast<size_t>(block.getNumSamples()));
                }

                std::vector<zlDelay::FIFODelay<SampleType> > &uDelay(uDelays[channel]);
                std::vector<zlDelay::FIFODelay<SampleType> > &vDelay(vDelays[channel]);

                auto currentBlock = block.getSingleChannelBlock(channel);
                for (size_t index = 0; index < static_cast<size_t>(block.getNumSamples()); ++index) {
                    {
                        const auto current = currentBlock.getSample(0, static_cast<int>(index));
                        u[index] = as[0] * current + uDelay[0].push(current);
                        v[index] = bs[0] * current;
                    }
                    for (size_t stage = 1; stage <= numStage; ++stage) {
                        const auto uCurrent = u[index];
                        const auto vCurrent = v[index];
                        u[index] = as[stage] * uCurrent - bs[stage] * vCurrent + uDelay[stage].push(uCurrent);
                        v[index] = bs[stage] * uCurrent + as[stage] * vCurrent + vDelay[stage].push(vCurrent);
                    }
                    currentBlock.setSample(0, static_cast<int>(index), u[index] + aB * v[index]);
                }
            }
        }
    };
}

#endif //REVERSE_CC_POLE_BASE_HPP
