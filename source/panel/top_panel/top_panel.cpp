// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "top_panel.hpp"

namespace zlPanel {
    TopPanel::TopPanel(PluginProcessor &processor,
                       zlInterface::UIBase &base,
                       UISettingPanel &panelToShow)
        : logoPanel(processor, base, panelToShow),
          swapIcon(juce::Drawable::createFromImageData(BinaryData::arrowlr_svg,
                                                       BinaryData::arrowlr_svgSize)),
          lrIcon(juce::Drawable::createFromImageData(BinaryData::leftright_svg,
                                                     BinaryData::leftright_svgSize)),
          msIcon(juce::Drawable::createFromImageData(BinaryData::midside_svg,
                                                     BinaryData::midside_svgSize)),
          lhIcon(juce::Drawable::createFromImageData(BinaryData::lowhigh_svg,
                                                     BinaryData::lowhigh_svgSize)),
          tsIcon(juce::Drawable::createFromImageData(BinaryData::transientsteady_svg,
                                                     BinaryData::transientsteady_svgSize)),
          psIcon(juce::Drawable::createFromImageData(BinaryData::peaksteady_svg,
                                                     BinaryData::peaksteady_svgSize)),
          swapButton("", base),
          splitBox({lrIcon.get(), msIcon.get(), lhIcon.get(), tsIcon.get(), psIcon.get()}, base),
          boxAttachment(processor.parameters, zlDSP::splitType::ID, splitBox.getBox(),
                        zlDSP::splitType::choices.size()) {
        attach({&swapButton.getButton()}, {zlDSP::swap::ID}, processor.parameters, buttonAttachments);
        addAndMakeVisible(logoPanel);

        swapButton.getLAF().enableShadow(false);
        swapButton.getLAF().setDrawable(swapIcon.get());
        addAndMakeVisible(swapButton);

        splitBox.getLAF().setImageScale(.66f);
        addAndMakeVisible(splitBox);
    }

    void TopPanel::resized() {
        juce::Grid grid;
        using Track = juce::Grid::TrackInfo;
        using Fr = juce::Grid::Fr;

        grid.templateRows = {Track(Fr(1))};
        grid.templateColumns = {
            Track(Fr(2)), Track(Fr(1)), Track(Fr(1))
        };
        grid.items = {
            juce::GridItem(logoPanel).withArea(1, 1),
            juce::GridItem(swapButton).withArea(1, 2),
            juce::GridItem(splitBox).withArea(1, 3),
        };

        const auto bound = getLocalBounds();
        // bound = uiBase.getRoundedShadowRectangleArea(bound, 0.5f * uiBase.getFontSize(), {});
        grid.performLayout(bound.toNearestInt());
    }
} // zlPanel
