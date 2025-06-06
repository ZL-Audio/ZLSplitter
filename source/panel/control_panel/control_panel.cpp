// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "control_panel.hpp"

namespace zlPanel {
    ControlPanel::ControlPanel(PluginProcessor &processor, zlInterface::UIBase &base)
        : processorRef(processor), uiBase(base),
          lrPanel(processor.parameters, uiBase),
          lhPanel(processor.parameters, uiBase),
          tsPanel(processor.parameters, uiBase),
          psPanel(processor.parameters, uiBase) {
        addChildComponent(lrPanel);
        addChildComponent(lhPanel);
        addChildComponent(tsPanel);
        addChildComponent(psPanel);
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
        psPanel.setBounds(getLocalBounds());
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
                lrPanel.setVisible(true);
                lhPanel.setVisible(false);
                tsPanel.setVisible(false);
                psPanel.setVisible(false);
                break;
            }
            case zlDSP::splitType::lhigh: {
                lhPanel.setVisible(true);
                tsPanel.setVisible(false);
                lrPanel.setVisible(false);
                psPanel.setVisible(false);
                break;
            }
            case zlDSP::splitType::tsteady: {
                tsPanel.setVisible(true);
                lrPanel.setVisible(false);
                lhPanel.setVisible(false);
                psPanel.setVisible(false);
                break;
            }
            case zlDSP::splitType::psteady: {
                psPanel.setVisible(true);
                tsPanel.setVisible(false);
                lrPanel.setVisible(false);
                lhPanel.setVisible(false);
                break;
            }
            case zlDSP::splitType::numSplit: {
                lrPanel.setVisible(false);
                lhPanel.setVisible(false);
                tsPanel.setVisible(false);
                psPanel.setVisible(false);
                break;
            }
        }
    }
} // zlPanel
