// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "compact_figure_combobox.hpp"

namespace zlInterface {
    CompactFigureCombobox::CompactFigureCombobox(const std::vector<juce::Drawable *> &icons, UIBase &base)
        : uiBase(base), boxLookAndFeel(base), animator{} {
        const auto menu = comboBox.getRootMenu();
        for (size_t i = 0; i < icons.size(); ++i) {
            menu->addItem(static_cast<int>(i + 1), "", true, false, icons[i]->createCopy());
        }
        boxLookAndFeel.setImages(icons);

        comboBox.setScrollWheelEnabled(false);
        comboBox.setInterceptsMouseClicks(false, false);
        comboBox.setLookAndFeel(&boxLookAndFeel);
        addAndMakeVisible(comboBox);

        setEditable(true);
    }


    CompactFigureCombobox::~CompactFigureCombobox() {
        animator.cancelAllAnimations(false);
        comboBox.setLookAndFeel(nullptr);
    }

    void CompactFigureCombobox::resized() {
        auto bound = getLocalBounds().toFloat();
        bound = bound.withSizeKeepingCentre(bound.getWidth(), juce::jmin(bound.getHeight(),
                                                                         uiBase.getFontSize() * 2.f));
        comboBox.setBounds(bound.toNearestInt());
    }

    void CompactFigureCombobox::mouseUp(const juce::MouseEvent &event) {
        comboBox.mouseUp(event);
    }

    void CompactFigureCombobox::mouseDown(const juce::MouseEvent &event) {
        comboBox.mouseDown(event);
    }

    void CompactFigureCombobox::mouseDrag(const juce::MouseEvent &event) {
        comboBox.mouseDrag(event);
    }

    void CompactFigureCombobox::mouseEnter(const juce::MouseEvent &event) {
        comboBox.mouseEnter(event);
        animator.cancelAnimation(animationId, false);
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto effect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {boxLookAndFeel.getBoxAlpha()}, {1.f}, 1000, friz::Parametric::kEaseInQuad)
        };
        effect->updateFn = [this](int, const auto &vals) {
            boxLookAndFeel.setBoxAlpha(vals[0]);
            comboBox.repaint();
        };
        animator.addAnimation(std::move(effect));
    }

    void CompactFigureCombobox::mouseExit(const juce::MouseEvent &event) {
        comboBox.mouseExit(event);
        animator.cancelAnimation(animationId, false);
        if (animator.getAnimation(animationId) != nullptr)
            return;
        auto effect{
            friz::makeAnimation<friz::Parametric, 1>(
                animationId, {boxLookAndFeel.getBoxAlpha()}, {0.f}, 1000, friz::Parametric::kEaseOutQuad)
        };
        effect->updateFn = [this](int, const auto &vals) {
            boxLookAndFeel.setBoxAlpha(vals[0]);
            comboBox.repaint();
        };
        animator.addAnimation(std::move(effect));
    }

    void CompactFigureCombobox::mouseMove(const juce::MouseEvent &event) {
        comboBox.mouseMove(event);
    }
}
