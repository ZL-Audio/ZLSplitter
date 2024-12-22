// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef SNAPPING_SLIDER_H
#define SNAPPING_SLIDER_H

#include <juce_gui_basics/juce_gui_basics.h>

namespace zlInterface {
    class SnappingSlider final : public juce::Slider {
    public:
        explicit SnappingSlider(UIBase &base) : juce::Slider(), uiBase(base) {
        }

        void mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel) override {
            w = wheel;
            w.deltaX *= uiBase.getWheelSensitivity(0);
            w.deltaY *= uiBase.getWheelSensitivity(0);
            if (e.mods.isCommandDown()) {
                w.deltaX *= uiBase.getWheelSensitivity(1);
                w.deltaY *= uiBase.getWheelSensitivity(1);
            }
            Slider::mouseWheelMove(e, w);
        }

    private:
        UIBase &uiBase;
        juce::MouseWheelDetails w{0.f, 0.f, true, true, true};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnappingSlider)
    };
}
#endif //SNAPPING_SLIDER_H
