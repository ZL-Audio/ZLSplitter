// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef REVERSE_REAL_POLE_BASE_HPP
#define REVERSE_REAL_POLE_BASE_HPP

#include <juce_dsp/juce_dsp.h>

#include "fifo_delay.hpp"

namespace zlReverseIIR {
    template<typename SampleType>
    class ReverseRealPoleBase {
    public:
        explicit ReverseRealPoleBase(const size_t stage) {
            setNumStage(stage);
        }

        void setNumStage(const size_t stage) {
            numStage = stage;
            cs.resize(stage + 1);
        }

        void reset() {
            std::fill(cs.begin(), cs.end(), SampleType(0));
            for (auto &delay: delays) {
                for (auto &d: delay) {
                    d.reset();
                }
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            delays.resize(static_cast<size_t>(spec.numChannels));
            for (auto &delay: delays) {
                int numDelay = 1;
                delay.resize(numStage + 1);
                for (auto &d: delay) {
                    d.setMaximumDelayInSamples(static_cast<size_t>(numDelay));
                    d.setDelay(static_cast<size_t>(numDelay));
                    numDelay *= 2;
                }
            }
            reset();
        }

        void process(juce::AudioBuffer<SampleType> &buffer) {
            process(juce::dsp::AudioBlock<SampleType>(buffer));
        }

        void process(juce::dsp::AudioBlock<SampleType> block) {
            for (size_t channel = 0; channel < static_cast<size_t>(block.getNumChannels()); ++channel) {
                std::vector<FIFODelay<SampleType>> &delay(delays[channel]);
                auto currentBlock = block.getChannelPointer(channel);
                for (int index = 0; index < static_cast<int>(block.getNumSamples()); ++index) {
                    for (size_t stage = 0; stage <= numStage; ++stage) {
                        const auto current = currentBlock[index];
                        currentBlock[index] = cs[stage] * current + delay[stage].push(current);
                    }
                }
            }
        }

        void updateFromBiquad(const std::array<double, 2> &a) {
            cs[0] = -static_cast<SampleType>(a[1]);
            for (size_t i = 1; i <= numStage; ++i) {
                cs[i] = cs[i - 1] * cs[i - 1];
            }
        }

        [[nodiscard]] inline int getLatency() const {
            return (1 << (1 + static_cast<int>(numStage))) + 1;
        }

    private:
        size_t numStage;

        std::vector<std::vector<FIFODelay<SampleType>>> delays;
        std::vector<SampleType> cs;
    };
}

#endif //REVERSE_REAL_POLE_BASE_HPP
