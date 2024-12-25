// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROLLER_ATTACH_HPP
#define CONTROLLER_ATTACH_HPP

#include "controller.hpp"

namespace zlDSP {
    class ControllerAttach final : private juce::AudioProcessorValueTreeState::Listener,
                                   private::juce::AsyncUpdater {
    public:
        explicit ControllerAttach(juce::AudioProcessor &processor,
                                  juce::AudioProcessorValueTreeState &parameters,
                                  Controller &controller);

        ~ControllerAttach() override;

        void prepare(const juce::dsp::ProcessSpec &spec);

    private:
        juce::AudioProcessor &processorRef;
        juce::AudioProcessorValueTreeState &parametersRef;
        Controller &controllerRef;

        constexpr static std::array IDs{
            splitType::ID, mix::ID, swap::ID,
            lhFilterType::ID, lhSlope::ID, lhFreq::ID,
            tsSeparation::ID, tsBalance::ID, tsHold::ID, tsSmooth::ID,
            psAttack::ID, psBalance::ID, psHold::ID, psSmooth::ID
        };

        constexpr static std::array defaultVs{
            static_cast<float>(splitType::defaultI),
            mix::defaultV, static_cast<float>(swap::defaultV),
            static_cast<float>(lhFilterType::defaultI), static_cast<float>(lhSlope::defaultI), lhFreq::defaultV,
            tsSeparation::defaultV, tsBalance::defaultV, tsHold::defaultV, tsSmooth::defaultV,
            psAttack::defaultV, psBalance::defaultV, psHold::defaultV, psSmooth::defaultV
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        void initDefaults();
    };
} // zlDSP

#endif //CONTROLLER_ATTACH_HPP
