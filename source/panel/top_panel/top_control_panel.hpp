// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../PluginProcessor.hpp"
#include "../../gui/gui.hpp"
#include "../multilingual/tooltip_helper.hpp"
#include "../helper/helper.hpp"
#include "BinaryData.h"

namespace zlpanel {
    class TopControlPanel final : public juce::Component {
    public:
        explicit TopControlPanel(PluginProcessor &p, zlgui::UIBase &base,
                                 multilingual::TooltipHelper &tooltip_helper);

        int getIdealWidth() const;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        std::unique_ptr<juce::Drawable> swap_drawable_;
        zlgui::button::CompactButton swap_button_;
        zlgui::attachment::ButtonAttachment<true> swap_attach_;

        std::unique_ptr<juce::Drawable> bypass_drawable_;
        zlgui::button::CompactButton bypass_button_;
        zlgui::attachment::ButtonAttachment<true> bypass_attach_;
    };
}