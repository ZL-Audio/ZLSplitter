// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#if (MSVC)
#include "ipps.h"
#endif

#include "zlp/zlp.hpp"
#include "state/state.hpp"

class PluginProcessor : public juce::AudioProcessor {
public:
    zlstate::DummyProcessor dummy_processor_;
    juce::AudioProcessorValueTreeState parameters_, state_;

    PluginProcessor();

    ~PluginProcessor() override;

    void prepareToPlay(double sample_rate, int samples_per_block) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    void processBlock(juce::AudioBuffer<double> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;

    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String &new_name) override;

    void getStateInformation(juce::MemoryBlock &dest_data) override;

    void setStateInformation(const void *data, int size_in_bytes) override;

    bool supportsDoublePrecisionProcessing() const override { return true; }

private:
    std::atomic<float> &swap_ref_;
    std::array<std::vector<float>, 4> float_out_buffer;
    std::array<float*, 2> float_in_pointers{};
    std::array<float*, 4> float_out_pointers1{};
    std::array<float*, 4> float_out_pointers2{};

    std::array<std::vector<double>, 4> double_out_buffer;
    std::array<double*, 2> double_in_pointers{};
    std::array<double*, 4> double_out_pointers1{};
    std::array<double*, 4> double_out_pointers2{};

    zlp::Controller<float> float_controller_;
    zlp::ControllerAttach<float> float_controller_attach_;

    zlp::Controller<double> double_controller_;
    zlp::ControllerAttach<double> double_controller_attach_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};
