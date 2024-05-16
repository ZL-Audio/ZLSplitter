// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROLLER_ATTACH_HPP
#define CONTROLLER_ATTACH_HPP

#include "controller.hpp"

namespace zlDSP {
    class ControllerAttach : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ControllerAttach(juce::AudioProcessorValueTreeState &parameters,
                                  Controller &controller);

        ~ControllerAttach() override;

    private:
        juce::AudioProcessorValueTreeState &parametersRef;
        Controller &controllerRef;

        constexpr static std::array IDs{
            splitType::ID, mix::ID, swap::ID,
            lhSlope::ID, lhFreq::ID
        };

        void parameterChanged(const juce::String &parameterID, float newValue) override;
    };
} // zlDSP

#endif //CONTROLLER_ATTACH_HPP
