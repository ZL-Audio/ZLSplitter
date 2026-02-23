// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../gui/gui.hpp"

namespace zlpanel {
    class CreditPanel final : public juce::Component {
    public:
        explicit CreditPanel(zlgui::UIBase& base);

        void paint(juce::Graphics& g) override;

        int getIdeatlHeight() const;

    private:
        zlgui::UIBase& base_;

        static constexpr auto kText =
            "ZL Splitter is Free and Open-source. ZL Splitter is licensed under AGPLv3, except for the logo of ZL Audio and the logo of ZL Splitter. You can obtain the corresponding source code at https://github.com/ZL-Audio/ZLSplitter or https://gitee.com/ZL-Audio/ZLSplitter.\n\n"
            "Copyright (c) 2024 - [zsliu98](https://github.com/zsliu98)\n\n"
            "JUCE framework from [JUCE](https://github.com/juce-framework/JUCE)\n\n"
            "JUCE template from [pamplejuce](https://github.com/sudara/pamplejuce)\n\n"
            "[kfr](https://github.com/kfrlib/kfr) by [KFR](https://github.com/kfrlib)\n\n"
            "[Material Symbols](https://github.com/google/material-design-icons) by [Google](https://github.com/google)\n\n"
            "[inter](https://github.com/rsms/inter) by [The Inter Project Authors](https://github.com/rsms/inter)\n\n"
            "Vicanek, Martin. A New Reverse IIR Filtering Algorithm. (2022).\n\n"
            "Redmon, Nigel. Cascading filters. (2016).\n\n"
            "Fitzgerald, Derry. Harmonic/percussive separation using median filtering. (2010).";

        [[nodiscard]] juce::TextLayout getTipTextLayout(const juce::String& text,
                                                        const float w, const float h) const {
            juce::AttributedString s;
            s.setJustification(juce::Justification::topLeft);
            s.append(text, juce::FontOptions(base_.getFontSize() * 1.5f), base_.getTextColour());
            juce::TextLayout tl;
            tl.createLayout(s, w, h);
            return tl;
        }
    };
}
