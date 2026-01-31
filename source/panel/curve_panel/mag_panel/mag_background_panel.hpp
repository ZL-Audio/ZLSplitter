// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../../PluginProcessor.hpp"
#include "../../../gui/gui.hpp"
#include "../../../state/state.hpp"
#include "../../helper/helper.hpp"

namespace zlpanel {
    class MagBackgroundPanel final : public juce::Component {
    public:
        explicit MagBackgroundPanel(PluginProcessor &p, zlgui::UIBase &base);

        void paint(juce::Graphics &g) override;

        void resized() override;

        void repaintCallBackSlow();

    private:
        zlgui::UIBase &base_;
        zlgui::attachment::ComponentUpdater updater_;

        zlgui::combobox::CompactCombobox mag_time_length_box_;
        zlgui::attachment::ComboBoxAttachment<true> mag_time_length_attach_;

        zlgui::combobox::CompactCombobox min_db_box_;
        zlgui::attachment::ComboBoxAttachment<true> min_db_attach_;

        zlgui::combobox::CompactCombobox mag_type_box_;
        zlgui::attachment::ComboBoxAttachment<true> mag_type_attach_;

        int c_mag_min_db_{-1};
    };
}