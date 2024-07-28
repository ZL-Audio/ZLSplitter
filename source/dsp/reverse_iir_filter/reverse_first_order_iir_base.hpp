// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef REVERSE_FIRST_ORDER_IIR_BASE_HPP
#define REVERSE_FIRST_ORDER_IIR_BASE_HPP

#include "reverse_real_pole_base.hpp"

namespace zlReverseIIR {
    /**
     * reverse of a first order IIR
     * @tparam SampleType
     */
    template<typename SampleType>
    class ReverseFirstOrderIIRBase {
    public:
        explicit ReverseFirstOrderIIRBase(const size_t stage) : reversePole(stage) {
        }

        void setNumStage(const size_t stage) {
            reversePole.setNumStage(stage);
        }

        void reset() {
            reversePole.reset();
            std::fill(states.begin(), states.end(), SampleType(0));
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
                auto currentBlock = block.getChannelPointer(channel);
                for (size_t index = 0; index < static_cast<size_t>(block.getNumSamples()); ++index) {
                    const auto current = currentBlock[index];
                    currentBlock[index] = zeros[1] * current + zeros[0] * states[channel];
                    states[channel] = current;
                }
            }
        }

        void updateFromBiquad(const std::array<double, 2> &a, const std::array<double, 2> &b) {
            const auto a0Inv = 1.0 / a[0];
            reversePole.updateFromBiquad({1.0, a[1] * a0Inv});
            zeros[0] = static_cast<SampleType>(b[0] * a0Inv);
            zeros[1] = static_cast<SampleType>(b[1] * a0Inv);
        }

        [[nodiscard]] inline int getLatency() const {
            return reversePole.getLatency();
        }

    private:
        ReverseRealPoleBase<SampleType> reversePole;
        std::vector<SampleType> states;
        std::array<SampleType, 2> zeros;
    };
}

#endif //REVERSE_FIRST_ORDER_IIR_BASE_HPP
