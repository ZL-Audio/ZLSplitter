// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../helper/helper.hpp"
#include "../../multilingual/tooltip_helper.hpp"

namespace zlpanel {
    class PSPopPanel final : public juce::Component {
    public:
        PSPopPanel(PluginProcessor &p, zlgui::UIBase &base,
                   multilingual::TooltipHelper &tooltip_helper);

        int getIdealHeight() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_{};

        zlgui::slider::CompactLinearSlider<true, true, true> balance_slider_;
        zlgui::attachment::SliderAttachment<true> balance_attach_;

        zlgui::slider::CompactLinearSlider<true, true, true> attack_slider_;
        zlgui::attachment::SliderAttachment<true> attack_attach_;

        zlgui::slider::CompactLinearSlider<true, true, true> hold_slider_;
        zlgui::attachment::SliderAttachment<true> hold_attach_;

        zlgui::slider::CompactLinearSlider<true, true, true> smooth_slider_;
        zlgui::attachment::SliderAttachment<true> smooth_attach_;
    };
}