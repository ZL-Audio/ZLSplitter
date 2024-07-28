// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef COMPACT_FIGURE_COMBOBOX_LOOK_AND_FEEL_H
#define COMPACT_FIGURE_COMBOBOX_LOOK_AND_FEEL_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "../../interface_definitions.hpp"

namespace zlInterface {
    class CompactFigureComboboxLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        // rounded menu box
        explicit CompactFigureComboboxLookAndFeel(UIBase &base) : uiBase(base) {
            setColour(juce::PopupMenu::backgroundColourId, uiBase.getBackgroundInactiveColor());
        }

        void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int, int, int, int,
                          juce::ComboBox &box) override {
            juce::ignoreUnused(width, height);
            const auto boxBounds = juce::Rectangle<float>(0.f, 0.f,
                                                          static_cast<float>(width),
                                                          static_cast<float>(height));
            const auto cornerSize = uiBase.getFontSize() * 0.375f;
            if (isButtonDown || box.isPopupActive()) {
                g.setColour(uiBase.getTextInactiveColor());
                g.fillRoundedRectangle(boxBounds, cornerSize);
            } else {
                uiBase.fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize,
                                                       {
                                                           .blurRadius = 0.45f, .flip = true,
                                                           .mainColour = uiBase.getBackgroundColor().
                                                           withMultipliedAlpha(
                                                               juce::jlimit(.25f, .5f, boxAlpha.load())),
                                                           .darkShadowColor = uiBase.getDarkShadowColor().
                                                           withMultipliedAlpha(boxAlpha.load()),
                                                           .brightShadowColor = uiBase.getBrightShadowColor().
                                                           withMultipliedAlpha(boxAlpha.load()),
                                                           .changeMain = true, .changeDark = true, .changeBright = true
                                                       });
            }
            const auto imageId = static_cast<size_t>(box.getSelectedItemIndex());
            if (imageId < images.size() && images[imageId] != nullptr) {
                const auto tempDrawable = images[imageId]->createCopy();
                tempDrawable->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
                const auto imageBound = boxBounds.withSizeKeepingCentre(
                    boxBounds.getWidth() * imageScale.load(),
                    uiBase.getFontSize() * fontScale * imageScale.load());
                const auto opacity = editable.load() ? 1.f : .5f;
                tempDrawable->drawWithin(g, imageBound, juce::RectanglePlacement::Flags::centred, opacity);
            }
        }

        void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override {
            juce::ignoreUnused(box);
            label.setBounds({0, 0, 0, 0});
        }

        void drawPopupMenuBackground(juce::Graphics &g, int width, int height) override {
            const auto cornerSize = uiBase.getFontSize() * 0.375f;
            const auto boxBounds = juce::Rectangle<float>(0, 0, static_cast<float>(width),
                                                          static_cast<float>(height));
            uiBase.fillRoundedInnerShadowRectangle(g, boxBounds, cornerSize, {.blurRadius = 0.45f, .flip = true});
        }

        void getIdealPopupMenuItemSize(const juce::String &text, const bool isSeparator, int standardMenuItemHeight,
                                       int &idealWidth, int &idealHeight) override {
            juce::ignoreUnused(text, isSeparator, standardMenuItemHeight);
            idealWidth = static_cast<int>(0);
            idealHeight = static_cast<int>(uiBase.getFontSize() * fontScale * 1.2f);
        }

        void drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area,
                               const bool isSeparator, const bool isActive,
                               const bool isHighlighted, const bool isTicked, const bool hasSubMenu,
                               const juce::String &text,
                               const juce::String &shortcutKeyText, const juce::Drawable *icon,
                               const juce::Colour *const textColourToUse) override {
            juce::ignoreUnused(text, isSeparator, hasSubMenu, shortcutKeyText, textColourToUse);
            float opacity = 1.f;
            if ((isHighlighted || isTicked) && isActive && editable) {
                opacity = 1.f;
            } else if (!isActive) {
                opacity = .5f * .25f;
            } else {
                opacity = .5f;
            }
            const auto imageBound = area.toFloat().withSizeKeepingCentre(
                static_cast<float>(area.getWidth()) * imageScale.load(),
                uiBase.getFontSize() * fontScale * imageScale.load());
            const auto tempDrawable = icon->createCopy();
            tempDrawable->replaceColour(juce::Colour(0, 0, 0), uiBase.getTextColor());
            tempDrawable->drawWithin(g, imageBound, juce::RectanglePlacement::Flags::centred, opacity);
        }

        int getMenuWindowFlags() override {
            return 1;
        }

        int getPopupMenuBorderSize() override {
            return juce::roundToInt(uiBase.getFontSize() * 0.125f);
        }

        inline void setEditable(const bool f) { editable.store(f); }

        inline void setBoxAlpha(const float x) { boxAlpha.store(x); }

        inline void setFontScale(const float x) { fontScale.store(x); }

        inline float getBoxAlpha() const { return boxAlpha.load(); }

        inline void setImages(const std::vector<juce::Drawable *> &x) { images = x; }

        inline void setImageScale(const float x) { imageScale.store(x); }

    private:
        std::atomic<bool> editable = true;
        std::atomic<float> boxAlpha, fontScale = 1.5f, imageScale{1.f};

        UIBase &uiBase;
        std::vector<juce::Drawable *> images;
    };
}

#endif //COMPACT_FIGURE_COMBOBOX_LOOK_AND_FEEL_H
