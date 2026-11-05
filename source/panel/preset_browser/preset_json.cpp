// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "preset_json.hpp"

#include <cmath>
#include <limits>

#include "../../state/state_schema.hpp"

namespace zlpanel {
    namespace {
        constexpr auto kPresetFormat = "ZLSplitterPreset";
        constexpr juce::int64 kMaximumPresetBytes = 16 * 1024 * 1024;
        constexpr size_t kMaximumTreeDepth = 32;
        constexpr size_t kMaximumTreeNodes = 8192;

        bool hasBoundedJsonDepth(const juce::String& json) {
            size_t depth = 0;
            bool in_string = false, escaped = false;
            for (const auto character : json) {
                if (in_string) {
                    if (escaped) { escaped = false; }
                    else if (character == '\\') { escaped = true; }
                    else if (character == '"') { in_string = false; }
                } else if (character == '"') {
                    in_string = true;
                } else if (character == '{' || character == '[') {
                    if (++depth > 128) { return false; }
                } else if (character == '}' || character == ']') {
                    if (depth == 0) { return false; }
                    --depth;
                }
            }
            return depth == 0 && !in_string;
        }

        bool isScalarProperty(const juce::var& value) {
            return value.isString() || value.isBool() || value.isInt() || value.isInt64() ||
                (value.isDouble() && std::isfinite(static_cast<double>(value)));
        }

        juce::Result validateProcessorState(const juce::ValueTree& state) {
            if (!state.isValid() || !state.hasType(zlstate::schema::kProcessorState)) {
                return juce::Result::fail("Preset does not contain the expected processor state");
            }
            if (!state.getChildWithName(zlstate::schema::kParameterState).isValid() ||
                !state.getChildWithName(zlstate::schema::kNonAutomatableState).isValid()) {
                return juce::Result::fail("Preset processor state is incomplete");
            }
            return juce::Result::ok();
        }
    }

    juce::Result PresetJson::write(const juce::File& file, const juce::MemoryBlock& processor_state) {
        if (processor_state.isEmpty() || processor_state.getSize() > static_cast<size_t>(kMaximumPresetBytes)) {
            return juce::Result::fail("Processor state is empty");
        }

        const auto xml = juce::AudioProcessor::getXmlFromBinary(processor_state.getData(),
                                                                 static_cast<int>(processor_state.getSize()));
        if (xml == nullptr || !xml->hasTagName(zlstate::schema::kProcessorState)) {
            return juce::Result::fail("Processor state is invalid");
        }

        const auto state = juce::ValueTree::fromXml(*xml);
        if (const auto result = validateProcessorState(state); result.failed()) {
            return juce::Result::fail("Processor state is incomplete");
        }

        auto* document = new juce::DynamicObject();
        document->setProperty("format", kPresetFormat);
        document->setProperty("version", 1);
        document->setProperty("state", valueTreeToJson(state));

        if (!file.replaceWithText(juce::JSON::toString(juce::var{document}, false))) {
            return juce::Result::fail("Preset file could not be written");
        }
        return juce::Result::ok();
    }

    juce::Result PresetJson::read(const juce::File& file, juce::MemoryBlock& processor_state) {
        if (!file.existsAsFile() || file.getSize() > kMaximumPresetBytes) {
            return juce::Result::fail("Preset file is missing or too large");
        }
        const auto json = file.loadFileAsString();
        if (!hasBoundedJsonDepth(json)) {
            return juce::Result::fail("Preset JSON is invalid or nested too deeply");
        }
        juce::var document;
        if (const auto result = juce::JSON::parse(json, document); result.failed()) {
            return juce::Result::fail("Preset is not valid JSON");
        }

        const auto* object = document.getDynamicObject();
        if (object == nullptr || !object->getProperty("format").isString() ||
            object->getProperty("format").toString() != kPresetFormat ||
            !(object->getProperty("version").isInt() || object->getProperty("version").isInt64()) ||
            static_cast<juce::int64>(object->getProperty("version")) != 1) {
            return juce::Result::fail("Preset format is not supported");
        }

        juce::ValueTree state;
        size_t remaining_nodes = kMaximumTreeNodes;
        if (const auto result = jsonToValueTree(object->getProperty("state"), state, 0, remaining_nodes); result.failed()) {
            return result;
        }
        if (const auto result = validateProcessorState(state); result.failed()) {
            return result;
        }

        const auto xml = state.createXml();
        if (xml == nullptr) {
            return juce::Result::fail("Preset state could not be restored");
        }
        juce::MemoryBlock restored;
        juce::AudioProcessor::copyXmlToBinary(*xml, restored);
        if (restored.isEmpty()) { return juce::Result::fail("Preset state is empty"); }
        processor_state.swapWith(restored);
        return juce::Result::ok();
    }

    juce::var PresetJson::valueTreeToJson(const juce::ValueTree& tree) {
        auto* object = new juce::DynamicObject();
        object->setProperty("type", tree.getType().toString());

        auto* properties = new juce::DynamicObject();
        for (auto index = 0; index < tree.getNumProperties(); ++index) {
            const auto name = tree.getPropertyName(index);
            properties->setProperty(name, tree.getProperty(name));
        }
        object->setProperty("properties", juce::var{properties});

        juce::Array<juce::var> children;
        children.ensureStorageAllocated(tree.getNumChildren());
        for (const auto& child : tree) {
            children.add(valueTreeToJson(child));
        }
        object->setProperty("children", juce::var{std::move(children)});
        return juce::var{object};
    }

    juce::Result PresetJson::jsonToValueTree(const juce::var& value, juce::ValueTree& tree,
                                           const size_t depth, size_t& remaining_nodes) {
        if (depth > kMaximumTreeDepth || remaining_nodes == 0) {
            return juce::Result::fail("Preset state is too complex");
        }
        --remaining_nodes;
        const auto* object = value.getDynamicObject();
        if (object == nullptr) {
            return juce::Result::fail("Preset contains an invalid state node");
        }

        const auto type = object->getProperty("type").toString();
        if (!object->getProperty("type").isString() || !juce::XmlElement::isValidXmlName(type)) {
            return juce::Result::fail("Preset state node has no type");
        }
        tree = juce::ValueTree{juce::Identifier{type}};

        const auto properties_value = object->getProperty("properties");
        const auto* properties_object = properties_value.getDynamicObject();
        if (properties_object == nullptr) {
            return juce::Result::fail("Preset state node has invalid properties");
        }
        const auto& properties = properties_object->getProperties();
        for (auto index = 0; index < properties.size(); ++index) {
            if (!juce::XmlElement::isValidXmlName(properties.getName(index).toString()) ||
                !isScalarProperty(properties.getValueAt(index))) {
                return juce::Result::fail("Preset state contains an invalid property");
            }
            tree.setProperty(properties.getName(index), properties.getValueAt(index), nullptr);
        }

        const auto children_value = object->getProperty("children");
        const auto* children = children_value.getArray();
        if (children == nullptr) {
            return juce::Result::fail("Preset state node has invalid children");
        }
        for (const auto& child_value : *children) {
            juce::ValueTree child;
            if (const auto result = jsonToValueTree(child_value, child, depth + 1, remaining_nodes); result.failed()) {
                return result;
            }
            tree.appendChild(child, nullptr);
        }
        return juce::Result::ok();
    }
}
