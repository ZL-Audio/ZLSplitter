// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "meter_panel.hpp"

namespace zlPanel {
    MeterPanel::MeterPanel(PluginProcessor &processor, zlInterface::UIBase &base)
        : processorRef(processor), uiBase(base),
          meterP1(processor.getController().getMeter1(), base),
          meterP2(processor.getController().getMeter2(), base),
          label1("Label 1"), label2("Label 2"),
          labelLAF(base),
          currentT(juce::Time::getCurrentTime()),
          vblank(this, [this]() { repaintCallBack(); }) {
        label1.setLookAndFeel(&labelLAF);
        label2.setLookAndFeel(&labelLAF);
        splitType.store(static_cast<int>(processor.parameters.getRawParameterValue(zlDSP::splitType::ID)->load()));
        swap.store(static_cast<bool>(processor.parameters.getRawParameterValue(zlDSP::swap::ID)->load()));
        handleAsyncUpdate();
        processorRef.parameters.addParameterListener(zlDSP::splitType::ID, this);
        processorRef.parameters.addParameterListener(zlDSP::swap::ID, this);
    }

    MeterPanel::~MeterPanel() {
        processorRef.parameters.removeParameterListener(zlDSP::splitType::ID, this);
        processorRef.parameters.removeParameterListener(zlDSP::swap::ID, this);
    }

    void MeterPanel::resized() {
        auto bound = getLocalBounds().toFloat();
        auto labelBound = bound.removeFromTop(uiBase.getFontSize() * 2.f);
        label1.setBounds(labelBound.removeFromLeft(labelBound.getWidth() * .5f).toNearestInt());
        label2.setBounds(labelBound.toNearestInt());
        meterP1.setBounds(bound.removeFromLeft(bound.getWidth() * .5f).toNearestInt());
        meterP2.setBounds(bound.toNearestInt());
    }

    void MeterPanel::parameterChanged(const juce::String &parameterID, float newValue) {
        if (parameterID == zlDSP::splitType::ID) {
            splitType.store(static_cast<int>(newValue));
            triggerAsyncUpdate();
        } else if (parameterID == zlDSP::swap::ID) {
            swap.store(static_cast<bool>(newValue));
            triggerAsyncUpdate();
        }
    }

    void MeterPanel::handleAsyncUpdate() {
        const auto currentSplitType = static_cast<zlDSP::splitType::stype>(splitType.load());
        juce::String text1, text2;
        switch (currentSplitType) {
            case zlDSP::splitType::stype::lright: {
                text1 = "Left";
                text2 = "Right";
                break;
            }
            case zlDSP::splitType::stype::mside: {
                text1 = "Mid";
                text2 = "Side";
                break;
            }
            case zlDSP::splitType::stype::lhigh: {
                text1 = "Low";
                text2 = "High";
                break;
            }
            case zlDSP::splitType::stype::tsteady: {
                text1 = "Tran";
                text2 = "Stea";
                break;
            }
        }
        if (!swap.load()) {
            label1.setText(text1, juce::sendNotification);
            label2.setText(text2, juce::sendNotification);
        } else {
            label2.setText(text1, juce::sendNotification);
            label1.setText(text2, juce::sendNotification);
        }
    }

    void MeterPanel::repaintCallBack() {
        const juce::Time nowT = juce::Time::getCurrentTime();
        if ((nowT - currentT).inMilliseconds() >= 16) {
            meterP1.repaint();
            meterP2.repaint();
            currentT = nowT;
        }
    }
} // zlPanel
