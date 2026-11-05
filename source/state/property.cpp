// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "property.hpp"

namespace zlstate {
    Property::Property() {
    }

    Property::Property(juce::AudioProcessorValueTreeState& apvts) {
        loadAPVTS(apvts);
    }

    void Property::loadAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        std::lock_guard<std::mutex> lock_guard{mutex_};
        if (checkCreateDirectory()) {
            if (const auto xml = juce::XmlDocument::parse(kUIPath); xml) {
                const auto loaded_state = juce::ValueTree::fromXml(*xml);
                if (loaded_state.hasType(apvts.state.getType())) {
                    apvts.replaceState(loaded_state);
                }
            }
        }
    }

    void Property::saveAPVTS(juce::AudioProcessorValueTreeState& apvts) {
        std::lock_guard<std::mutex> lock{mutex_};
        if (checkCreateDirectory()) {
            if (const auto xml = apvts.copyState().createXml(); xml) {
                juce::TemporaryFile temporary(kUIPath);
                if (xml->writeTo(temporary.getFile())) {
                    (void)temporary.overwriteTargetFileWithTemporary();
                }
            }
        }
    }

    bool Property::checkCreateDirectory() const {
        // create directory if not exists
        if (!kPath.isDirectory()) {
            if (!kPath.createDirectory()) {
                return false;
            }
        }
        // check if UI preset exists
        if (kUIPath.existsAsFile()) {
            return true;
        }
        // check if old UI preset exists
        // yes -> copy old UI preset to UI preset
        if (kOldUIPath.existsAsFile()) {
            if (const auto c_res = kOldUIPath.copyFileTo(kUIPath); c_res) {
                return true;
            }
        }
        // no -> create a blank UI preset
        const auto res = kUIPath.create();
        return res.wasOk();
    }
}
