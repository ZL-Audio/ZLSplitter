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
    ControllerAttach::ControllerAttach(juce::AudioProcessor &processor, juce::AudioProcessorValueTreeState &parameters,
                                       Controller &controller)
        : processorRef(processor), parametersRef(parameters),
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
            sType.store(static_cast<splitType::stype>(newValue));
            triggerAsyncUpdate();
        } else if (parameterID == mix::ID) {
            controllerRef.setMix(static_cast<double>(newValue) / 200.0);
        } else if (parameterID == swap::ID) {
            controllerRef.setSwap(static_cast<bool>(newValue));
        } else if (parameterID == lhSlope::ID) {
            controllerRef.getLHSplitter().setOrder(lhSlope::orders[static_cast<size_t>(newValue)]);
        } else if (parameterID == lhFreq::ID) {
            controllerRef.getLHSplitter().setFreq(static_cast<double>(newValue));
        } else if (parameterID == tsSeperation::ID) {
            controllerRef.getTSSplitter(0).setFactor(newValue / 100.f);
            controllerRef.getTSSplitter(1).setFactor(newValue / 100.f);
        }
    }

    void ControllerAttach::handleAsyncUpdate() {
        switch (sType.load()) {
            case splitType::lright:
            case splitType::mside:
            case splitType::lhigh: {
                break;
            }
            case splitType::tsteady: {
                processorRef.setLatencySamples(controllerRef.getTSSplitter(0).getLatency());
            }
        }
    }
} // zlDSP
