// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "PluginEditor.hpp"
#include "state/state_schema.hpp"

PluginEditor::PluginEditor(PluginProcessor& p) :
    AudioProcessorEditor(&p),
    p_ref_(p),
    state_(dummy_processor_, nullptr,
           juce::Identifier(zlstate::schema::kUISettings),
           zlstate::getStateParameterLayout()),
    property_(state_),
    base_(state_),
    main_panel_(p, base_, static_cast<zlpanel::multilingual::TooltipLanguage>(base_.gui.getTooltipLangID())) {
    // set font
#if defined(JUCE_WINDOWS)
    base_.gui.font_ = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterSubsetMediumNoHinting_ttf, BinaryData::InterSubsetMediumNoHinting_ttfSize);
#else
    base_.gui.font_ = juce::Typeface::createSystemTypefaceFor(
        BinaryData::InterSubsetMedium_ttf, BinaryData::InterSubsetMedium_ttfSize);
#endif
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(base_.gui.font_);

    // add the main panel
    addAndMakeVisible(main_panel_);
    main_panel_.addMouseListener(this, true);

    // set size & size listener
    setResizeLimits(static_cast<int>(zlstate::PWindowW::kMinV),
                    static_cast<int>(zlstate::PWindowH::kMinV),
                    static_cast<int>(zlstate::PWindowW::kMaxV),
                    static_cast<int>(zlstate::PWindowH::kMaxV));
    setResizable(true, p.wrapperType != PluginProcessor::wrapperType_AudioUnitv3);

    this->resizableCorner = std::make_unique<zlgui::ResizeCorner>(base_.gui, this, getConstrainer(),
                                                                  zlgui::ResizeCorner::kScaleWithWidth, 0.016f);
    addChildComponent(this->resizableCorner.get());
    this->resizableCorner->setAlwaysOnTop(true);
    this->resizableCorner->resized();

    last_ui_width_.referTo(state_.getParameterAsValue(zlstate::PWindowW::kID));
    last_ui_height_.referTo(state_.getParameterAsValue(zlstate::PWindowH::kID));
    setSize(std::clamp(static_cast<int>(last_ui_width_.getValue()), zlstate::PWindowW::kMinV, zlstate::PWindowW::kMaxV),
            std::clamp(static_cast<int>(last_ui_height_.getValue()), zlstate::PWindowH::kMinV, zlstate::PWindowH::kMaxV));

    startTimer(kVisibilityTimer, 500);
    updateIsShowing();

    base_.setPanelProperty(zlgui::kUISettingChanged, true);
    base_.getPanelValueTree().addListener(this);
    triggerAsyncUpdate();
}

PluginEditor::~PluginEditor() {
    main_panel_.removeMouseListener(this);
    base_.getPanelValueTree().removeListener(this);
    flushPendingPropertySave();
    vblank_.reset();
    stopTimer(kVisibilityTimer);
    p_ref_.getController().setAnalyzerOn(false);
}

void PluginEditor::paint(juce::Graphics& g) {
    juce::ignoreUnused(g);
}

void PluginEditor::resized() {
    main_panel_.setBounds(getLocalBounds());
    if (!base_.getWindowSizeFix()) {
        const auto width = getWidth();
        const auto height = getHeight();
        const auto size_changed = width != static_cast<int>(last_ui_width_.getValue()) ||
            height != static_cast<int>(last_ui_height_.getValue());
        last_ui_width_ = width;
        last_ui_height_ = height;
        if (size_changed) {
            schedulePropertySave();
        }
    }
    triggerAsyncUpdate();
}

void PluginEditor::visibilityChanged() {
    updateIsShowing();
}

void PluginEditor::parentHierarchyChanged() {
    updateIsShowing();
}

void PluginEditor::minimisationStateChanged(bool) {
    updateIsShowing();
}

void PluginEditor::valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier& property) {
    if (base_.isPanelIdentifier(zlgui::kUISettingChanged, property)) {
        triggerAsyncUpdate();
        schedulePropertySave();
    }
}

void PluginEditor::handleAsyncUpdate() {
    main_panel_.resized();
    sendLookAndFeelChange();
}

void PluginEditor::timerCallback(const int timer_id) {
    if (timer_id == kVisibilityTimer) {
        updateIsShowing();
    } else if (timer_id == kPropertySaveTimer) {
        flushPendingPropertySave();
    }
}

void PluginEditor::schedulePropertySave() {
    startTimer(kPropertySaveTimer, kPropertySaveDelayMS);
}

void PluginEditor::flushPendingPropertySave() {
    if (isTimerRunning(kPropertySaveTimer)) {
        stopTimer(kPropertySaveTimer);
        property_.saveAPVTS(state_);
    }
}

void PluginEditor::updateIsShowing() {
    if (isShowing() != base_.getIsEditorShowing()) {
        base_.setIsEditorShowing(isShowing());
        p_ref_.getController().setAnalyzerOn(base_.getIsEditorShowing());
        if (base_.getIsEditorShowing()) {
            vblank_ = std::make_unique<juce::VBlankAttachment>(
                &main_panel_, [this](const double x) {
                    main_panel_.repaintCallBack(x);
                });
        } else {
            vblank_.reset();
        }
    }
}

int PluginEditor::getControlParameterIndex(Component& c) {
    for (auto* component = &c; component != nullptr && component != this;
         component = component->getParentComponent()) {
        if (const auto parameter = p_ref_.parameters_.getParameter(component->getComponentID()); parameter != nullptr) {
            return parameter->getParameterIndex();
        }
    }
    return -1;
}

void PluginEditor::mouseDown(const juce::MouseEvent& event) {
    if (!event.mods.isRightButtonDown() || event.getNumberOfClicks() != 1) {
        return;
    }
    for (auto* component = event.originalComponent; component != nullptr && component != this;
         component = component->getParentComponent()) {
        if (const auto parameter = p_ref_.parameters_.getParameter(component->getComponentID()); parameter != nullptr) {
            if (const auto* context = getHostContext(); context != nullptr) {
                if (const auto menu = context->getContextMenuForParameter(parameter)) {
                    menu->showNativeMenu(juce::Component::getMouseXYRelative());
                }
            }
            return;
        }
    }
}
