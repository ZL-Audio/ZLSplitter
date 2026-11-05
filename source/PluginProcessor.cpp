// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include "state/state_schema.hpp"
#include <type_traits>

//==============================================================================
PluginProcessor::PluginProcessor() :
    AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output 1", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output 2", juce::AudioChannelSet::stereo(), true)),
    parameters_(*this, nullptr,
                juce::Identifier(zlstate::schema::kParameterState),
                zlp::getParameterLayout()),
    na_parameters_(dummy_processor_, nullptr,
                   juce::Identifier(zlstate::schema::kNonAutomatableState),
                   zlstate::getNAParameterLayout()),
    swap_ref_(*parameters_.getRawParameterValue(zlp::PSwap::kID)),
    bypass_ref_(*parameters_.getRawParameterValue(zlp::PBypass::kID)),
    double_controller_(*this),
    double_controller_attach_(*this, parameters_, double_controller_) {
}

PluginProcessor::~PluginProcessor() = default;

//==============================================================================
const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    return 1;
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return "Default";
}

void PluginProcessor::changeProgramName(int, const juce::String&) {
}

//==============================================================================
void PluginProcessor::prepareToPlay(const double sample_rate, const int samples_per_block) {
    const auto max_num_samples = static_cast<size_t>(std::max(1, samples_per_block));
    sample_rate_.store(sample_rate, std::memory_order::relaxed);

    for (size_t chan = 0; chan < 2; ++chan) {
        double_in_buffer[chan].resize(max_num_samples);
        double_in_pointers[chan] = double_in_buffer[chan].data();
    }
    for (size_t chan = 0; chan < 4; ++chan) {
        double_out_buffer[chan].resize(max_num_samples);
        double_out_pointers1[chan] = double_out_buffer[chan].data();
    }
    double_out_pointers2[0] = double_out_pointers1[2];
    double_out_pointers2[1] = double_out_pointers1[3];
    double_out_pointers2[2] = double_out_pointers1[0];
    double_out_pointers2[3] = double_out_pointers1[1];
    double_controller_.prepare(sample_rate, max_num_samples);
}

void PluginProcessor::releaseResources() {
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }
    const auto aux_out = layouts.getChannelSet(false, 1);
    if (!aux_out.isDisabled() && aux_out != juce::AudioChannelSet::stereo()) {
        return false;
    }
    return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    if (bypass_ref_.load(std::memory_order::relaxed) < .5f) {
        processBlockInternal<false>(buffer);
    } else {
        processBlockInternal<true>(buffer);
    }
}

void PluginProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer&) {
    if (bypass_ref_.load(std::memory_order::relaxed) < .5f) {
        processBlockInternal<false>(buffer);
    } else {
        processBlockInternal<true>(buffer);
    }
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    processBlockInternal<true>(buffer);
}

void PluginProcessor::processBlockBypassed(juce::AudioBuffer<double>& buffer, juce::MidiBuffer&) {
    processBlockInternal<true>(buffer);
}

template <bool IsBypassed, typename FloatType>
void PluginProcessor::processBlockInternal(juce::AudioBuffer<FloatType>& buffer) {
    juce::ScopedNoDenormals no_denormals;
    const auto count = static_cast<size_t>(buffer.getNumSamples());
    if (count == 0) {
        return;
    }
    for (size_t chan = 0; chan < 2; ++chan) {
        if constexpr (std::is_same_v<FloatType, double>) {
            double_in_pointers[chan] = buffer.getWritePointer(static_cast<int>(chan));
        } else {
            double_in_pointers[chan] = double_in_buffer[chan].data();
            zldsp::vector::copy(double_in_pointers[chan], buffer.getReadPointer(static_cast<int>(chan)), count);
        }
    }
    auto& outputs = swap_ref_.load(std::memory_order::relaxed) < .5f
        ? double_out_pointers1 : double_out_pointers2;
    double_controller_.process(double_in_pointers, outputs, count);
    // update the dry delay after processing for continuous bypass
    double_controller_.processBypassDelay(double_in_pointers, count);
    if constexpr (!IsBypassed) {
        for (int chan = 0; chan < std::min(4, buffer.getNumChannels()); ++chan) {
            zldsp::vector::copy(buffer.getWritePointer(chan),
                               double_out_pointers1[static_cast<size_t>(chan)], count);
        }
    } else {
        if constexpr (!std::is_same_v<FloatType, double>) {
            for (size_t chan = 0; chan < 2; ++chan) {
                zldsp::vector::copy(buffer.getWritePointer(static_cast<int>(chan)), double_in_pointers[chan], count);
            }
        }
        for (int chan = 2; chan < buffer.getNumChannels(); ++chan) {
            buffer.clear(chan, 0, buffer.getNumSamples());
        }
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true;
}

juce::AudioProcessorEditor* PluginProcessor::createEditor() {
    return new PluginEditor(*this);
}

//==============================================================================
void PluginProcessor::getStateInformation(juce::MemoryBlock& dest_data) {
    auto temp_tree = juce::ValueTree(zlstate::schema::kProcessorState);
    temp_tree.appendChild(parameters_.copyState(), nullptr);
    temp_tree.appendChild(na_parameters_.copyState(), nullptr);
    const std::unique_ptr<juce::XmlElement> xml(temp_tree.createXml());
    copyXmlToBinary(*xml, dest_data);
}

void PluginProcessor::setStateInformation(const void* data, const int size_in_bytes) {
    std::unique_ptr<juce::XmlElement> xml_state(getXmlFromBinary(data, size_in_bytes));
    if (xml_state != nullptr && xml_state->hasTagName(zlstate::schema::kProcessorState)) {
        const auto temp_tree = juce::ValueTree::fromXml(*xml_state);
        const auto parameter_state = temp_tree.getChildWithName(parameters_.state.getType());
        const auto non_automatable_state = temp_tree.getChildWithName(na_parameters_.state.getType());
        if (!parameter_state.isValid() || !non_automatable_state.isValid()) {
            return;
        }
        parameters_.replaceState(parameter_state);
        na_parameters_.replaceState(non_automatable_state);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor*JUCE_CALLTYPE createPluginFilter() {
    return new PluginProcessor();
}
