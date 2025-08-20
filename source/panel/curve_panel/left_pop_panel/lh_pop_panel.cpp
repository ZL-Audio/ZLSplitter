// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "lh_pop_panel.hpp"

namespace zlpanel {
    LHPopPanel::LHPopPanel(PluginProcessor &p, zlgui::UIBase &base)
        : base_(base), updater_(),
          mix_slider_("Mix", base),
          mix_attach_(mix_slider_.getSlider(), p.parameters_,
                      zlp::PMix::kID, updater_),
          filter_type_box_(zlp::PLHFilterType::kChoices, base),
          filter_type_attach_(filter_type_box_.getBox(), p.parameters_,
                              zlp::PLHFilterType::kID, updater_),
          filter_slope_box_(zlp::PLHSlope::kChoices, base),
          filter_slope_attach_(filter_slope_box_.getBox(), p.parameters_,
                               zlp::PLHSlope::kID, updater_),
          freq_slider_("Freq", base),
          freq_attach_(freq_slider_.getSlider1(), p.parameters_,
                       zlp::PLHFreq::kID, updater_) {
        addAndMakeVisible(mix_slider_);
        addAndMakeVisible(filter_type_box_);
        addAndMakeVisible(filter_slope_box_);
        addAndMakeVisible(freq_slider_);

        setInterceptsMouseClicks(false, true);

        setBufferedToImage(true);
    }

    int LHPopPanel::getIdealHeight() const {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);
        return (padding + slider_height) * 2 + slider_width;
    }

    void LHPopPanel::resized() {
        const auto padding = juce::roundToInt(base_.getFontSize() * kPaddingScale);
        // const auto slider_width = juce::roundToInt(base_.getFontSize() * kSliderScale);
        const auto slider_height = juce::roundToInt(base_.getFontSize() * kSliderHeightScale);

        auto bound = getLocalBounds(); {
            mix_slider_.setBounds(bound.removeFromTop(slider_height));
            bound.removeFromTop(padding);
        } {
            auto temp_bound = bound.removeFromTop(slider_height);
            auto left_bound = temp_bound.removeFromLeft(temp_bound.getWidth() / 2);
            left_bound = left_bound.withSizeKeepingCentre(left_bound.getWidth() - (padding / 2) * 2, left_bound.getHeight());
            filter_type_box_.setBounds(left_bound);
            const auto right_bound = temp_bound.withSizeKeepingCentre(left_bound.getWidth(), left_bound.getHeight());
            filter_slope_box_.setBounds(right_bound);
            bound.removeFromTop(padding);
        }
        {
            freq_slider_.setBounds(bound);
        }
    }

    void LHPopPanel::repaintCallBackSlow() {
        updater_.updateComponents();
    }
}
