// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMBOBOX_SUB_ATTACHMENT_HPP
#define COMBOBOX_SUB_ATTACHMENT_HPP

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    class ComboBoxSubAttachment final : private juce::ComboBox::Listener {
    public:
        ComboBoxSubAttachment(const juce::AudioProcessorValueTreeState &stateToUse,
                              const juce::String &parameterID,
                              juce::ComboBox &combo,
                              const int numItem)
            : comboBox(combo),
              storedParameter(*(stateToUse.getParameter(parameterID))),
              numItems(numItem),
              attachment(*(stateToUse.getParameter(parameterID)),
                         [this](const float f) { setValue(f); },
                         stateToUse.undoManager) {
            sendInitialUpdate();
            comboBox.addListener(this);
        }

        ~ComboBoxSubAttachment() override {
            comboBox.removeListener(this);
        }

        /** Call this after setting up your combo box in the case where you need to do
            extra setup after constructing this attachment.
        */
        void sendInitialUpdate() {
            attachment.sendInitialUpdate();
        }

    private:
        void setValue(const float newValue) {
            const auto normValue = storedParameter.convertTo0to1(newValue);
            const auto index = std::min(juce::roundToInt(normValue * static_cast<float>(numItems - 1)),
                                        comboBox.getNumItems() - 1);

            if (index == comboBox.getSelectedItemIndex()) return;

            const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
            comboBox.setSelectedItemIndex(index, juce::sendNotificationSync);
        }

        void comboBoxChanged(juce::ComboBox *) override {
            if (ignoreCallbacks)
                return;

            const auto selected = static_cast<float>(comboBox.getSelectedItemIndex());
            const auto newValue = numItems > 1
                                      ? selected / static_cast<float>(numItems - 1)
                                      : 0.0f;
            attachment.setValueAsCompleteGesture(storedParameter.convertFrom0to1(newValue));
        }

        juce::ComboBox &comboBox;
        juce::RangedAudioParameter &storedParameter;
        int numItems;
        juce::ParameterAttachment attachment;
        bool ignoreCallbacks = false;
    };
}

#endif //COMBOBOX_SUB_ATTACHMENT_HPP
