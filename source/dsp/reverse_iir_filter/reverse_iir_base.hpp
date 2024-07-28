// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef REVERSE_IIR_BASE_HPP
#define REVERSE_IIR_BASE_HPP

#include "reverse_cc_pole_base.hpp"

namespace zlReverseIIR {
    template<typename SampleType>
    class ReverseIIRBase {
    public:
        explicit ReverseIIRBase(const size_t stage) : reversePole(stage) {}

        void setNumStage(const size_t stage) {
            reversePole.setNumStage(stage);
        }

        void reset() {
            reversePole.reset();
            for (auto &state : states) {
                std::fill(state.begin(), state.end(), SampleType(0));
            }
        }

        void prepare(const juce::dsp::ProcessSpec &spec) {
            reversePole.prepare(spec);
            states.resize(static_cast<size_t>(spec.numChannels));
            reset();
        }

        void process(juce::AudioBuffer<SampleType> &buffer) {
            process(juce::dsp::AudioBlock<SampleType>(buffer));
        }

        void process(juce::dsp::AudioBlock<SampleType> block) {
            reversePole.process(block);
            for (size_t channel = 0; channel < static_cast<size_t>(block.getNumChannels()); ++channel) {
                std::array<SampleType, 2> &state(states[channel]);
                auto currentBlock = block.getChannelPointer(channel);
                for (size_t index = 0; index < static_cast<size_t>(block.getNumSamples()) ; ++index) {
                    const auto current = currentBlock[index];
                    currentBlock[index] = zeros[2] * current + zeros[1] * state[0] + zeros[2] * state[1];
                    state[1] = state[0];
                    state[0] = current;
                }
            }
        }

        void updateFromBiquad(const std::array<double, 3> &a, const std::array<double, 3> &b) {
            const auto a0Inv = 1.0 / a[0];
            reversePole.updateFromBiquad({1.0, a[1] * a0Inv, a[2] * a0Inv});
            zeros[0] = static_cast<SampleType>(b[0] * a0Inv);
            zeros[1] = static_cast<SampleType>(b[1] * a0Inv);
            zeros[2] = static_cast<SampleType>(b[2] * a0Inv);
        }

        [[nodiscard]] inline int getLatency() const {
            return reversePole.getLatency();
        }

    private:
        ReverseCCPoleBase<SampleType> reversePole;
        std::vector<std::array<SampleType, 2>> states;
        std::array<SampleType, 3> zeros;
    };
}

#endif //REVERSE_IIR_BASE_HPP
