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
        initDefaults();
        for (auto &ID: IDs) {
            parametersRef.addParameterListener(ID, this);
        }
    }

    ControllerAttach::~ControllerAttach() {
        for (auto &ID: IDs) {
            parametersRef.removeParameterListener(ID, this);
        }
    }

    void ControllerAttach::prepare(const juce::dsp::ProcessSpec &spec) {
        triggerAsyncUpdate();
    }

    void ControllerAttach::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == splitType::ID) {
            controllerRef.setType(static_cast<splitType::stype>(newValue));
            triggerAsyncUpdate();
        } else if (parameterID == mix::ID) {
            controllerRef.setMix(static_cast<double>(newValue) / 200.0);
        } else if (parameterID == swap::ID) {
            controllerRef.setSwap(static_cast<bool>(newValue));
        } else if (parameterID == lhFilterType::ID) {
            controllerRef.setLHFilterType(static_cast<lhFilterType::ftype>(newValue));
            triggerAsyncUpdate();
        } else if (parameterID == lhSlope::ID) {
            controllerRef.getLHSplitter().setOrder(lhSlope::orders[static_cast<size_t>(newValue)]);
            controllerRef.getLHLinearSplitter().setOrder(lhSlope::orders[static_cast<size_t>(newValue)]);
            triggerAsyncUpdate();
        } else if (parameterID == lhFreq::ID) {
            controllerRef.getLHSplitter().setFreq(static_cast<double>(newValue));
            controllerRef.getLHLinearSplitter().setFreq(static_cast<double>(newValue));
        } else if (parameterID == tsBalance::ID) {
            controllerRef.getTSSplitter(0).setBalance(tsBalance::formatV(newValue));
            controllerRef.getTSSplitter(1).setBalance(tsBalance::formatV(newValue));
        } else if (parameterID == tsSeperation::ID) {
            controllerRef.getTSSplitter(0).setSeperation(tsSeperation::formatV(newValue));
            controllerRef.getTSSplitter(1).setSeperation(tsSeperation::formatV(newValue));
        } else if (parameterID == tsHold::ID) {
            controllerRef.getTSSplitter(0).setHold(tsHold::formatV(newValue));
            controllerRef.getTSSplitter(1).setHold(tsHold::formatV(newValue));
        } else if (parameterID == tsSmooth::ID) {
            controllerRef.getTSSplitter(0).setSmooth(tsSmooth::formatV(newValue));
            controllerRef.getTSSplitter(1).setSmooth(tsSmooth::formatV(newValue));
        }
    }

    void ControllerAttach::handleAsyncUpdate() {
        processorRef.setLatencySamples(controllerRef.getLatency());
    }

    void ControllerAttach::initDefaults() {
        for (size_t j = 0; j < defaultVs.size(); ++j) {
            parameterChanged(IDs[j], defaultVs[j]);
        }
    }
} // zlDSP
