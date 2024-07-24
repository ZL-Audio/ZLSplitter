// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"

#include "panel/main_panel.hpp"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::AudioProcessorValueTreeState::Listener,
                     private juce::AsyncUpdater {
public:
    explicit PluginEditor(PluginProcessor &);

    ~PluginEditor() override;

    void paint(juce::Graphics &) override;

    void resized() override;

private:
    PluginProcessor &processorRef;
    zlState::Property property;
    std::atomic<bool> isSizeChanged{false};

    zlPanel::MainPanel mainPanel;

    juce::Value lastUIWidth, lastUIHeight;

    constexpr const static std::array IDs{
        zlState::windowW::ID,
        zlState::windowH::ID,
        zlState::wheelSensitivity::ID, zlState::wheelFineSensitivity::ID,
        zlState::rotaryStyle::ID, zlState::rotaryDragSensitivity::ID
    };

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void handleAsyncUpdate() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
