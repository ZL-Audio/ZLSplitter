// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "top_choice_panel.hpp"

namespace zlpanel {
    TopChoicePanel::TopChoicePanel(PluginProcessor& p, zlgui::UIState& base,
                                   const multilingual::TooltipHelper& tooltip_helper) :
        p_ref_(p), base_(base),
        analyzer_type_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PAnalyzerShow::kID)),
        analyzer_type_buttons_{
            zlgui::button::ClickTextButton(base.gui, "FFT",
                                           tooltip_helper.getToolTipText(multilingual::kFFTAnalyzer)),
            zlgui::button::ClickTextButton(base.gui, "MAG",
                                           tooltip_helper.getToolTipText(multilingual::kMagAnalyzer)),
            zlgui::button::ClickTextButton(base.gui, "WAV")
        } {
        for (size_t i = 0; i < analyzer_type_buttons_.size(); ++i) {
            analyzer_type_buttons_[i].getButton().onClick = [this, i]() {
                auto* parameter = p_ref_.na_parameters_.getParameter(zlstate::PAnalyzerShow::kID);
                const auto value = parameter->convertTo0to1(static_cast<float>(i));
                if (std::abs(value - parameter->getValue()) > 1e-6f) {
                    parameter->beginChangeGesture();
                    parameter->setValueNotifyingHost(value);
                    parameter->endChangeGesture();
                }
                repaintCallBackSlow();
            };
        }
        for (auto& button : analyzer_type_buttons_) {
            button.getLAF().setJustification(juce::Justification::centred);
            button.getLAF().setFontScale(1.5f);
            button.getButton().setToggleable(true);
            button.getButton().setClickingTogglesState(false);
            button.setBufferedToImage(true);
            addAndMakeVisible(button);
        }
        setBufferedToImage(true);
        setInterceptsMouseClicks(false, true);
    }

    int TopChoicePanel::getIdealWidth() const {
        const auto button_width = juce::roundToInt(base_.gui.getFontSize() * kButtonScale * 2.f);
        return 3 * button_width;
    }

    void TopChoicePanel::resized() {
        const auto button_width = juce::roundToInt(base_.gui.getFontSize() * kButtonScale * 2.f);
        auto bound = getLocalBounds();
        for (auto& button : analyzer_type_buttons_) {
            button.setBounds(bound.removeFromLeft(button_width));
        }
    }

    void TopChoicePanel::repaintCallBackSlow() {
        const auto new_analyzer_type = analyzer_type_ref_.load(std::memory_order::relaxed);
        if (std::abs(new_analyzer_type - c_analyzer_type_) > .1f) {
            c_analyzer_type_ = new_analyzer_type;
            for (size_t i = 0; i < analyzer_type_buttons_.size(); ++i) {
                const auto f = std::abs(static_cast<float>(i) - c_analyzer_type_) < .1f;
                analyzer_type_buttons_[i].getButton().setToggleState(f, juce::dontSendNotification);
            }
        }
    }
}
