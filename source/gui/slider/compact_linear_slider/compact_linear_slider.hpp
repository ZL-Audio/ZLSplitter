// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPACT_LINEAR_SLIDER_H
#define COMPACT_LINEAR_SLIDER_H

#include <friz/friz.h>

#include "../../label/name_look_and_feel.hpp"
#include "compact_linear_slider_look_and_feel.hpp"
#include "../extra_slider/snapping_slider.h"

namespace zlInterface {
    class CompactLinearSlider : public juce::Component {
    public:
        explicit CompactLinearSlider(const juce::String &labelText, UIBase &base);

        ~CompactLinearSlider() override;

        void resized() override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        void mouseDoubleClick(const juce::MouseEvent &event) override;

        void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        inline juce::Slider &getSlider() { return slider; }

        inline NameLookAndFeel &getLabelLAF() { return nameLookAndFeel; }

        inline void setEditable(const bool x) {
            nameLookAndFeel.setEditable(x);
            textLookAndFeel.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline void setPadding(const float lr, const float ub) {
            lrPad.store(lr);
            ubPad.store(ub);
        }

        inline void setFontScale(const float x1, const float x2) {
            nameLookAndFeel.setFontScale(x1);
            textLookAndFeel.setFontScale(x2);
        }

    private:
        UIBase &uiBase;

        CompactLinearSliderLookAndFeel sliderLookAndFeel;
        NameLookAndFeel nameLookAndFeel, textLookAndFeel;
        SnappingSlider slider;
        juce::Label label, text;

        friz::Animator animator{};
        static constexpr int animationId = 1;

        std::atomic<float> lrPad = 0, ubPad = 0;

        juce::String getDisplayValue(juce::Slider &s);
    };
}

#endif //COMPACT_LINEAR_SLIDER_H
