// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef LR_PANEL_HPP
#define LR_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "../../gui/gui.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {
    class LRPanel final : public juce::Component {
    public:
        explicit LRPanel(juce::AudioProcessorValueTreeState &parameter, zlInterface::UIBase &base);

        void resized() override;

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::TwoValueRotarySlider mixS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;
    };
} // zlPanel

#endif //LR_PANEL_HPP
