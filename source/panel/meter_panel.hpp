// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef METER_PANEL_HPP
#define METER_PANEL_HPP

#include <juce_gui_basics/juce_gui_basics.h>

#include "../PluginProcessor.h"
#include "../dsp/dsp.hpp"
#include "../gui/gui.hpp"
#include "meter_panel/single_meter_panel.hpp"

namespace zlPanel {
    class MeterPanel final : public juce::Component,
                             private juce::AudioProcessorValueTreeState::Listener,
                             private::juce::AsyncUpdater {
    public:
        explicit MeterPanel(PluginProcessor &processor, zlInterface::UIBase &base);

        ~MeterPanel() override;

        void resized() override;

    private:
        PluginProcessor &processorRef;
        zlInterface::UIBase &uiBase;
        SingleMeterPanel meterP1, meterP2;
        juce::Label label1, label2;
        zlInterface::NameLookAndFeel labelLAF;

        std::atomic<int> splitType;
        std::atomic<bool> swap;

        constexpr static std::array dspIDs{zlDSP::splitType::ID, zlDSP::swap::ID};

        void parameterChanged(const juce::String &parameterID, float newValue) override;

        void handleAsyncUpdate() override;

        juce::Time currentT;
        juce::VBlankAttachment vblank;

        void repaintCallBack();
    };
} // zlPanel

#endif //METER_PANEL_HPP
