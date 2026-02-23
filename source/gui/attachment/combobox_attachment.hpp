// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "component_updater.hpp"

namespace zlgui::attachment {
    template <bool kUpdateFromAPVTS = true>
    class ComboBoxAttachment final : public ComponentAttachment,
                                     private juce::AudioProcessorValueTreeState::Listener,
                                     private juce::ComboBox::Listener {
    public:
        /**
         *
         * @param box
         * @param apvts
         * @param parameter_ID
         * @param updater
         * @param forward_map map a parameter int to a box item index
         * @param backward_map map a box item index to a parameter int
         * @param notification_type
         */
        ComboBoxAttachment(juce::ComboBox& box,
                           juce::AudioProcessorValueTreeState& apvts,
                           const juce::String& parameter_ID,
                           ComponentUpdater& updater,
                           const std::vector<int> forward_map = {},
                           const std::vector<int> backward_map = {},
                           const juce::NotificationType notification_type =
                               juce::NotificationType::sendNotificationSync) :
            box_(box), notification_type_(notification_type),
            apvts_(apvts), parameter_ref_(*apvts_.getParameter(parameter_ID)),
            updater_ref_(updater), forward_map_(forward_map), backward_map_(backward_map) {
            jassert(forward_map.size() == backward_map.size());
            jassert(forward_map_.size() == box.getNumItems());
            // add combobox listener
            box_.addListener(this);
            // add parameter listener
            if constexpr (kUpdateFromAPVTS) {
                apvts_.addParameterListener(parameter_ref_.getParameterID(), this);
            }
            parameterChanged(parameter_ref_.getParameterID(),
                             apvts_.getRawParameterValue(
                                 parameter_ref_.getParameterID())->load(std::memory_order::relaxed));
            if constexpr (kUpdateFromAPVTS) {
                updater_ref_.addAttachment(*this);
            } else {
                updateComponent();
            }
        }

        ~ComboBoxAttachment() override {
            if constexpr (kUpdateFromAPVTS) {
                updater_ref_.removeAttachment(*this);
                apvts_.removeParameterListener(parameter_ref_.getParameterID(), this);
            }
            box_.removeListener(this);
        }

        void updateComponent() override {
            auto para_index = atomic_index_.load(std::memory_order::relaxed);
            auto box_index = box_.getSelectedItemIndex();
            if (!backward_map_.empty() && box_index >= 0) {
                box_index = backward_map_[static_cast<size_t>(box_index)];
            }
            if (para_index != box_index) {
                if (!forward_map_.empty()) {
                    para_index = forward_map_[static_cast<size_t>(para_index)];
                }
                box_.setSelectedItemIndex(para_index, notification_type_);
            }
        }

    private:
        juce::ComboBox& box_;
        juce::NotificationType notification_type_{juce::NotificationType::sendNotificationSync};
        juce::AudioProcessorValueTreeState& apvts_;
        juce::RangedAudioParameter& parameter_ref_;
        ComponentUpdater& updater_ref_;
        std::atomic<int> atomic_index_{0};
        std::vector<int> forward_map_{};
        std::vector<int> backward_map_{};

        void parameterChanged(const juce::String&, const float new_value) override {
            atomic_index_.store(static_cast<int>(new_value), std::memory_order::relaxed);
            updater_ref_.getFlag().store(true, std::memory_order::release);
        }

        void comboBoxChanged(juce::ComboBox*) override {
            parameter_ref_.beginChangeGesture();
            auto box_index = box_.getSelectedItemIndex();
            if (!backward_map_.empty() && box_index >= 0) {
                box_index = backward_map_[static_cast<size_t>(box_index)];
            }
            parameter_ref_.setValueNotifyingHost(
                parameter_ref_.convertTo0to1(static_cast<float>(box_index)));
            parameter_ref_.endChangeGesture();
        }
    };
}
