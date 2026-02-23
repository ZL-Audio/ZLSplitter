// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "logo_panel.hpp"
#include "top_legend_panel.hpp"
#include "top_choice_panel.hpp"

namespace zlpanel {
    class TopPanel final : public juce::Component {
    public:
        explicit TopPanel(PluginProcessor &p, zlgui::UIBase &base,
                          multilingual::TooltipHelper &tooltip_helper);

        void paint(juce::Graphics &g) override;

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        LogoPanel logo_panel_;
        TopLegendPanel top_legend_panel_;
        TopChoicePanel top_choice_panel_;

        zlgui::combobox::CompactCombobox split_type_box_;
        zlgui::attachment::ComboBoxAttachment<true> split_type_attachment_;

        std::unique_ptr<juce::Drawable> swap_drawable_;
        zlgui::button::ClickButton swap_button_;
        zlgui::attachment::ButtonAttachment<true> swap_attach_;

        std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::ClickButton bypass_button_;
        zlgui::attachment::ButtonAttachment<true> bypass_attach_;
    };
}