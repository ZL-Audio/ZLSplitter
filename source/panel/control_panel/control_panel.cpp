// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

namespace zlPanel {
    ControlPanel::ControlPanel(PluginProcessor &processor, zlInterface::UIBase &base)
        : processorRef(processor), uiBase(base),
          lrPanel(processor.parameters, uiBase),
          lhPanel(processor.parameters, uiBase),
          tsPanel(processor.parameters, uiBase) {
        addChildComponent(lrPanel);
        addChildComponent(lhPanel);
        addChildComponent(tsPanel);
        splitType.store(static_cast<int>(processorRef.parameters.getRawParameterValue(zlDSP::splitType::ID)->load()));
        handleAsyncUpdate();
        processorRef.parameters.addParameterListener(zlDSP::splitType::ID, this);
    }

    ControlPanel::~ControlPanel() {
        processorRef.parameters.removeParameterListener(zlDSP::splitType::ID, this);
    }

    void ControlPanel::resized() {
        lrPanel.setBounds(getLocalBounds());
        lhPanel.setBounds(getLocalBounds());
        tsPanel.setBounds(getLocalBounds());
    }

    void ControlPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        juce::ignoreUnused(parameterID);
        splitType.store(static_cast<int>(newValue));
        triggerAsyncUpdate();
    }

    void ControlPanel::handleAsyncUpdate() {
        switch (static_cast<zlDSP::splitType::stype>(splitType.load())) {
            case zlDSP::splitType::lright:
            case zlDSP::splitType::mside: {
                lhPanel.setVisible(false);
                tsPanel.setVisible(false);
                lrPanel.setVisible(true);
                break;
            }
            case zlDSP::splitType::lhigh: {
                tsPanel.setVisible(false);
                lrPanel.setVisible(false);
                lhPanel.setVisible(true);
                break;
            }
            case zlDSP::splitType::tsteady: {
                lrPanel.setVisible(false);
                lhPanel.setVisible(false);
                tsPanel.setVisible(true);
                break;
            }
        }
    }
} // zlPanel
