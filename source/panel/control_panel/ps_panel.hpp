// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef PS_PANEL_HPP
#define PS_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "../../gui/gui.hpp"
#include "../panel_definitons.hpp"

namespace zlPanel {

    class PSPanel final : public juce::Component {
    public:
        explicit PSPanel(juce::AudioProcessorValueTreeState &parameter, zlInterface::UIBase &base);

        void resized() override;

    private:
        zlInterface::UIBase &uiBase;
        zlInterface::CompactLinearSlider balanceS, attackS, holdS, smoothS;
        juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachments;
    };

} // zlPanel

#endif //PS_PANEL_HPP
