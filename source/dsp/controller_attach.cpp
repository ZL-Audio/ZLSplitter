// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#include "controller_attach.hpp"

namespace zlDSP {
    ControllerAttach::ControllerAttach(juce::AudioProcessorValueTreeState &parameters,
                                       Controller &controller)
        : parametersRef(parameters),
          controllerRef(controller) {
        for (auto &ID: IDs) {
            parametersRef.addParameterListener(ID, this);
        }
    }

    ControllerAttach::~ControllerAttach() {
        for (auto &ID: IDs) {
            parametersRef.removeParameterListener(ID, this);
        }
    }

    void ControllerAttach::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == splitType::ID) {
            controllerRef.setType(static_cast<splitType::stype>(newValue));
        } else if (parameterID == mix::ID) {
            controllerRef.setMix(static_cast<double>(newValue) / 200.0);
        }
    }
} // zlDSP