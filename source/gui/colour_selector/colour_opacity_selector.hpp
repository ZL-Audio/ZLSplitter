// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef COLOUR_OPACITY_SELECTOR_HPP
#define COLOUR_OPACITY_SELECTOR_HPP

#include "colour_selector.hpp"
#include "../slider/slider.hpp"

namespace zlInterface {
    class ColourOpacitySelector final : public juce::Component,
                                        private juce::Slider::Listener {
    public:
        enum Layout {
            horizontal,
            vertical
        };

        explicit ColourOpacitySelector(zlInterface::UIBase &base,
                                       juce::Component &parent,
                                       bool useOpacity = true,
                                       float widthS = 12.f, float heightS = 10.f,
                                       float w1 = 0.3f, float w2 = 0.3f);

        ~ColourOpacitySelector() override;

        void resized() override;

        juce::Colour getColour() const {
            return selector.getColour();
        }

        void setColour(const juce::Colour c) {
            selector.setColour(c);
            slider.getSlider().setValue(static_cast<double>(c.getFloatAlpha()));
        }

    private:
        zlInterface::UIBase &uiBase;
        ColourSelector selector;
        zlInterface::CompactLinearSlider slider;
        bool opacityON;
        std::array<float, 2> weights{};

        void sliderValueChanged(juce::Slider *s) override;
    };
} // zlInterface

#endif //COLOUR_OPACITY_SELECTOR_HPP
