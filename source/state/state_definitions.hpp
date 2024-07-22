// Copyright (C) 2024 - zsliu98
// This file is part of ZLEqualizer
//
// ZLEqualizer is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLEqualizer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLEqualizer. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_STATE_DEFINITIONS_H
#define ZL_STATE_DEFINITIONS_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <BinaryData.h>

namespace zlState {
    inline auto static constexpr versionHint = 1;

    inline auto static constexpr bandNUM = 16;

    // float
    template<class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::ID + suffix, versionHint),
                                                               T::name + suffix, T::range, T::defaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::range.convertTo0to1(x);
        }
    };

    // bool
    template<class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::ID + suffix, versionHint),
                                                              T::name + suffix, T::defaultV, attributes);
        }

        inline static float convertTo01(const bool x) {
            return x ? 1.f : 0.f;
        }
    };

    // choice
    template<class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string &suffix = "", bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::name);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::ID + suffix, versionHint),
                                                                T::name + suffix, T::choices, T::defaultI, attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::choices.size() - 1);
        }
    };

    inline std::string appendSuffix(const std::string &s, const size_t i) {
        const auto suffix = i < 10 ? "0" + std::to_string(i) : std::to_string(i);
        return s + suffix;
    }

    class windowW : public FloatParameters<windowW> {
    public:
        auto static constexpr ID = "window_w";
        auto static constexpr name = "NA";
        inline static constexpr float minV = 600.f;
        inline static constexpr float maxV = 6000.f;
        inline static constexpr float defaultV = 704.f;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class windowH : public FloatParameters<windowH> {
    public:
        auto static constexpr ID = "window_h";
        auto static constexpr name = "NA";
        inline static constexpr float minV = 375.f;
        inline static constexpr float maxV = 3750.f;
        inline static constexpr float defaultV = 440.f;
        inline auto static const range =
                juce::NormalisableRange<float>(minV, maxV, 1.f);
    };

    class wheelSensitivity : public FloatParameters<wheelSensitivity> {
    public:
        auto static constexpr ID = "wheel_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr defaultV = 1.f;
    };

    class wheelFineSensitivity : public FloatParameters<wheelFineSensitivity> {
    public:
        auto static constexpr ID = "wheel_fine_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 1.f, 0.01f);
        auto static constexpr defaultV = .12f;
    };

    class rotaryStyle : public ChoiceParameters<rotaryStyle> {
    public:
        auto static constexpr ID = "rotary_style";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Circular", "Horizontal", "Vertical", "Horiz + Vert"
        };
        int static constexpr defaultI = 0;
        inline static std::array<juce::Slider::SliderStyle, 4> styles{
            juce::Slider::Rotary,
            juce::Slider::RotaryHorizontalDrag,
            juce::Slider::RotaryVerticalDrag,
            juce::Slider::RotaryHorizontalVerticalDrag
        };
    };

    class rotaryDragSensitivity : public FloatParameters<rotaryDragSensitivity> {
    public:
        auto static constexpr ID = "rotary_darg_sensitivity";
        auto static constexpr name = "";
        inline auto static const range = juce::NormalisableRange<float>(2.f, 32.f, 0.01f);
        auto static constexpr defaultV = 10.f;
    };

    inline void addOneColour(juce::AudioProcessorValueTreeState::ParameterLayout &layout,
                             const std::string &suffix = "",
                             const int red = 0, const int green = 0, const int blue = 0,
                             const bool addOpacity = false, const float opacity = 1.f) {
        layout.add(std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_r", versionHint), "",
                       0, 255, red),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_g", versionHint), "",
                       0, 255, green),
                   std::make_unique<juce::AudioParameterInt>(
                       juce::ParameterID(suffix + "_b", versionHint), "",
                       0, 255, blue));
        if (addOpacity) {
            layout.add(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID(suffix + "_o", versionHint), "",
                juce::NormalisableRange<float>(0.f, 1.f, .01f), opacity));
        }
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout getStateParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(windowW::get(), windowH::get(),
                   wheelSensitivity::get(), wheelFineSensitivity::get(),
                   rotaryStyle::get(), rotaryDragSensitivity::get());
        addOneColour(layout, "text", 255 - 8, 255 - 9, 255 - 11, true, 1.f);
        addOneColour(layout, "background", (255 - 214) / 2, (255 - 223) / 2, (255 - 236) / 2, true, 1.f);
        addOneColour(layout, "shadow", 0, 0, 0, true, 1.f);
        addOneColour(layout, "glow", 70, 66, 62, true, 1.f);
        return layout;
    }
}

#endif //ZL_STATE_DEFINITIONS_H
