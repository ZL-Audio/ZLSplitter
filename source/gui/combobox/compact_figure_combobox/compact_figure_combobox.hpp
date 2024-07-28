// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPACT_FIGURE_COMBOBOX_H
#define COMPACT_FIGURE_COMBOBOX_H

#include <friz/friz.h>

#include "compact_figure_combobox_look_and_feel.hpp"

namespace zlInterface {
    class CompactFigureCombobox final : public juce::Component {
    public:
        CompactFigureCombobox(const std::vector<juce::Drawable *> &icons, UIBase &base);

        ~CompactFigureCombobox() override;

        void resized() override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void mouseEnter(const juce::MouseEvent &event) override;

        void mouseExit(const juce::MouseEvent &event) override;

        void mouseMove(const juce::MouseEvent &event) override;

        inline void setEditable(const bool x) {
            boxLookAndFeel.setEditable(x);
            setInterceptsMouseClicks(x, false);
        }

        inline juce::ComboBox &getBox() { return comboBox; }

        inline CompactFigureComboboxLookAndFeel &getLAF() { return boxLookAndFeel; }

    private:
        zlInterface::UIBase &uiBase;
        CompactFigureComboboxLookAndFeel boxLookAndFeel;
        juce::ComboBox comboBox;

        friz::Animator animator;
        static constexpr int animationId = 1;
    };
}


#endif //COMPACT_FIGURE_COMBOBOX_H
