// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef CONTROL_PANEL_HPP
#define CONTROL_PANEL_HPP

#include "../../PluginProcessor.h"

#include "lr_panel.hpp"
#include "lh_panel.hpp"
#include "ts_panel.hpp"

namespace zlPanel {
    class ControlPanel final : public juce::Component,
                               private juce::AudioProcessorValueTreeState::Listener,
                               private juce::AsyncUpdater {
    public:
        explicit ControlPanel(PluginProcessor &processor, zlInterface::UIBase &base);

        ~ControlPanel() override;

        void resized() override;

    private:
        PluginProcessor &processorRef;
        zlInterface::UIBase &uiBase;

        LRPanel lrPanel;
        LHPanel lhPanel;
        TSPanel tsPanel;

        std::atomic<int> splitType;

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;
    };
} // zlPanel

#endif //CONTROL_PANEL_HPP
