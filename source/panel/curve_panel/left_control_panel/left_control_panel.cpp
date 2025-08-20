// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "left_control_panel.hpp"

namespace zlpanel {
    LeftControlPanel::LeftControlPanel(PluginProcessor &p, zlgui::UIBase &base)
        : p_ref_(p), base_(base),
          split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
          split_mode_drawables_{
              juce::Drawable::createFromImageData(BinaryData::leftright_svg, BinaryData::leftright_svgSize),
              juce::Drawable::createFromImageData(BinaryData::midside_svg, BinaryData::midside_svgSize),
              juce::Drawable::createFromImageData(BinaryData::lowhigh_svg, BinaryData::lowhigh_svgSize),
              juce::Drawable::createFromImageData(BinaryData::transientsteady_svg,
                                                  BinaryData::transientsteady_svgSize),
              juce::Drawable::createFromImageData(BinaryData::peaksteady_svg, BinaryData::peaksteady_svgSize)
          },
          split_mode_buttons_{
              zlgui::button::CompactButton("", base, ""),
              zlgui::button::CompactButton("", base, ""),
              zlgui::button::CompactButton("", base, ""),
              zlgui::button::CompactButton("", base, ""),
              zlgui::button::CompactButton("", base, "")
          } {
        juce::ignoreUnused(base_);
        for (size_t i = 0; i < split_mode_buttons_.size(); ++i) {
            auto &b{split_mode_buttons_[i]};
            b.getButton().onStateChange = [this, i]() {
                if (this->split_mode_buttons_[i].getButton().getToggleState()) {
                    auto *para = this->p_ref_.parameters_.getParameter(zlp::PSplitType::kID);
                    para->beginChangeGesture();
                    para->setValueNotifyingHost(para->convertTo0to1(static_cast<float>(i)));
                    para->endChangeGesture();

                    this->split_mode_buttons_[i].setInterceptsMouseClicks(false, false);
                } else {
                    this->split_mode_buttons_[i].setInterceptsMouseClicks(true, true);
                }
            };
        }
        for (size_t i = 0; i < split_mode_buttons_.size(); ++i) {
            auto &b{split_mode_buttons_[i]};
            b.setDrawable(split_mode_drawables_[i].get());
            b.getLAF().enableShadow(false);
            b.getLAF().setShrinkScale(.0f);
            b.getLAF().setScale(1.25f);
            b.setBufferedToImage(true);
            addAndMakeVisible(b);
        }

        setBufferedToImage(true);
    }

    void LeftControlPanel::resized() {
        auto bound = getLocalBounds();
        const auto button_height = bound.getHeight() / static_cast<int>(split_mode_buttons_.size());
        bound.removeFromTop((bound.getHeight() - button_height * static_cast<int>(split_mode_buttons_.size())) / 2);
        for (size_t i = 0; i < split_mode_buttons_.size(); ++i) {
            split_mode_buttons_[i].setBounds(bound.removeFromTop(button_height));
        }
    }

    void LeftControlPanel::repaintCallBackSlow() {
        const auto new_split_type = split_type_ref_.load(std::memory_order::relaxed);
        if (std::abs(new_split_type - c_split_type_) > .1f) {
            c_split_type_ = new_split_type;
            for (size_t i = 0; i < split_mode_buttons_.size(); ++i) {
                const auto f = std::abs(static_cast<float>(i) - c_split_type_) < .1f;
                split_mode_buttons_[i].getButton().setToggleState(f, juce::sendNotificationSync);
            }
        }
    }
}
