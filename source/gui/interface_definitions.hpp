// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZL_INTERFACE_DEFINES_H
#define ZL_INTERFACE_DEFINES_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "../state/state_definitions.hpp"

namespace zlInterface {
    enum colourIdx {
        textColour,
        backgroundColour,
        shadowColour,
        glowColour,
        colourNum
    };

    inline std::array<std::string, colourNum> colourNames{
        "text", "background",
        "shadow", "glow"
    };

    static constexpr size_t ColorMap1Size = 10;
    static constexpr size_t ColorMap2Size = 6;

    struct UIColors {
        juce::Colour ExtraColor1 = juce::Colour(139, 0, 0);
        std::array<juce::Colour, ColorMap1Size> ColorMap1 = {
            juce::Colour(31, 119, 180),
            juce::Colour(255, 127, 14),
            juce::Colour(44, 160, 44),
            juce::Colour(214, 39, 40),
            juce::Colour(148, 103, 189),
            juce::Colour(140, 86, 75),
            juce::Colour(227, 119, 194),
            juce::Colour(127, 127, 127),
            juce::Colour(188, 189, 34),
            juce::Colour(23, 190, 207)
        };
        std::array<juce::Colour, ColorMap2Size> ColorMap2 = {
            juce::Colour(76, 114, 176),
            juce::Colour(85, 168, 104),
            juce::Colour(196, 78, 82),
            juce::Colour(129, 114, 178),
            juce::Colour(255, 196, 0),
            juce::Colour(100, 181, 205),
        };
    };

    inline const std::array<UIColors, 2> styleColors{
        {
            {},
            {
                .ExtraColor1 = juce::Colour(255 - 139, 255, 255),
                .ColorMap1 = {
                    juce::Colour(224, 136, 75),
                    juce::Colour(0, 128, 241),
                    juce::Colour(211, 95, 211),
                    juce::Colour(41, 216, 215),
                    juce::Colour(107, 152, 66),
                    juce::Colour(115, 169, 180),
                    juce::Colour(28, 136, 61),
                    juce::Colour(128, 128, 128),
                    juce::Colour(67, 66, 221),
                    juce::Colour(232, 65, 48),
                },
                .ColorMap2 = {
                    juce::Colour(255, 192, 0),
                    juce::Colour(252, 18, 197),
                    juce::Colour(23, 255, 244),
                    juce::Colour(117, 212, 29),
                    juce::Colour(0, 59, 255),
                    juce::Colour(255, 40, 0),
                }
            }
        }
    };

    auto inline const TextColor = juce::Colour(87, 96, 110);
    auto inline const TextInactiveColor = TextColor.withAlpha(0.5f);
    auto inline const TextHideColor = TextColor.withAlpha(0.25f);

    auto inline const BackgroundColor = juce::Colour(214, 223, 236);
    auto inline const BackgroundInactiveColor = BackgroundColor.withAlpha(0.8f);
    auto inline const BackgroundHideColor = BackgroundColor.withAlpha(0.5f);
    auto inline const BackgroundInvisibleColor = BackgroundColor.withAlpha(0.25f);

    auto inline const DarkShadowColor = juce::Colour(168, 172, 178);
    auto inline const BrightShadowColor = juce::Colour(237, 246, 255);

    auto inline constexpr FontTiny = 0.5f;
    auto inline constexpr FontSmall = 0.75f;
    auto inline constexpr FontNormal = 1.0f;
    auto inline constexpr FontLarge = 1.25f;
    auto inline constexpr FontHuge = 1.5f;
    auto inline constexpr FontHuge2 = 3.0f;
    auto inline constexpr FontHuge3 = 4.5f;

    auto inline constexpr RefreshFreqHz = 60;

    struct fillRoundedShadowRectangleArgs {
        float blurRadius = 0.5f;
        bool curveTopLeft = true, curveTopRight = true, curveBottomLeft = true, curveBottomRight = true;
        bool fit = true, flip = false;
        bool drawBright = true, drawDark = true, drawMain = true;
        juce::Colour mainColour = BackgroundColor;
        juce::Colour darkShadowColor = DarkShadowColor;
        juce::Colour brightShadowColor = BrightShadowColor;
        bool changeMain = false, changeDark = false, changeBright = false;
    };

    struct fillShadowEllipseArgs {
        float blurRadius = 0.5f;
        bool fit = true, flip = false;
        bool drawBright = true, drawDark = true;
        juce::Colour mainColour = BackgroundColor;
        juce::Colour darkShadowColor = DarkShadowColor;
        juce::Colour brightShadowColor = BrightShadowColor;
        bool changeMain = false, changeDark = false, changeBright = false;
    };

    inline std::string formatFloat(float x, int precision) {
        std::stringstream stream;
        precision = std::max(0, precision);
        stream << std::fixed << std::setprecision(precision) << x;
        return stream.str();
    }

    inline std::string fixFormatFloat(float x, int length) {
        auto y = std::abs(x);
        if (y < 10) {
            return formatFloat(x, length - 1);
        } else if (y < 100) {
            return formatFloat(x, length - 2);
        } else if (y < 1000) {
            return formatFloat(x, length - 3);
        } else if (y < 10000) {
            return formatFloat(x, length - 4);
        } else {
            return formatFloat(x, length - 5);
        }
    }

    class UIBase {
    public:
        explicit UIBase(juce::AudioProcessorValueTreeState &apvts)
            : state(apvts), fontSize{0.f} {
        }

        UIBase(const float fSize, juce::AudioProcessorValueTreeState &apvts)
            : state(apvts) {
            fontSize.store(fSize);
        }

        void setFontSize(const float fSize) { fontSize.store(fSize); }

        inline float getFontSize() const { return fontSize.load(); }

        inline juce::Colour getTextColor() const {
            return customColours[static_cast<size_t>(textColour)];
        }

        inline juce::Colour getTextInactiveColor() const { return getTextColor().withAlpha(0.5f); }

        inline juce::Colour getTextHideColor() const { return getTextColor().withAlpha(0.25f); }

        inline juce::Colour getBackgroundColor() const {
            return customColours[static_cast<size_t>(backgroundColour)];
        }

        inline juce::Colour getBackgroundInactiveColor() const { return getBackgroundColor().withAlpha(0.8f); }

        inline juce::Colour getBackgroundHideColor() const { return getBackgroundColor().withAlpha(0.5f); }

        inline juce::Colour getDarkShadowColor() const {
            return customColours[static_cast<size_t>(shadowColour)];
        }

        inline juce::Colour getBrightShadowColor() const {
            return customColours[static_cast<size_t>(glowColour)];
        }

        inline juce::Colour getExtraColor1() const { return styleColors[1].ExtraColor1; }

        inline juce::Colour getColorMap1(const size_t idx) const {
            return styleColors[1].ColorMap1[idx % ColorMap1Size];
        }

        inline juce::Colour getColorMap2(const size_t idx) const {
            return styleColors[1].ColorMap2[idx % ColorMap2Size];
        }

        static juce::Rectangle<float> getRoundedShadowRectangleArea(juce::Rectangle<float> boxBounds,
                                                                    float cornerSize,
                                                                    const fillRoundedShadowRectangleArgs &margs);

        juce::Rectangle<float> fillRoundedShadowRectangle(juce::Graphics &g,
                                                          juce::Rectangle<float> boxBounds,
                                                          float cornerSize,
                                                          const fillRoundedShadowRectangleArgs &margs) const;

        juce::Rectangle<float> fillRoundedInnerShadowRectangle(juce::Graphics &g,
                                                               juce::Rectangle<float> boxBounds,
                                                               float cornerSize,
                                                               const fillRoundedShadowRectangleArgs &margs) const;

        static juce::Rectangle<float> getShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                           float cornerSize,
                                                           const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawShadowEllipse(juce::Graphics &g,
                                                 juce::Rectangle<float> boxBounds,
                                                 float cornerSize,
                                                 const fillShadowEllipseArgs &margs) const;

        static juce::Rectangle<float> getInnerShadowEllipseArea(juce::Rectangle<float> boxBounds,
                                                                float cornerSize,
                                                                const fillShadowEllipseArgs &margs);

        juce::Rectangle<float> drawInnerShadowEllipse(juce::Graphics &g,
                                                      juce::Rectangle<float> boxBounds,
                                                      float cornerSize,
                                                      const fillShadowEllipseArgs &margs) const;

        juce::Colour getColourByIdx(const colourIdx idx) const {
            return customColours[static_cast<size_t>(idx)];
        }

        void setColourByIdx(const colourIdx idx, const juce::Colour colour) {
            customColours[static_cast<size_t>(idx)] = colour;
        }

        inline float getWheelSensitivity(const size_t x) const {
            return wheelSensitivity[x];
        }

        void setWheelSensitivity(const float v, const size_t x) {
            wheelSensitivity[x] = v;
        }

        size_t getRotaryStyleID() const {
            return rotaryStyleId;
        }

        juce::Slider::SliderStyle getRotaryStyle() const {
            return zlState::rotaryStyle::styles[rotaryStyleId];
        }

        void setRotaryStyleID(const size_t x) {
            rotaryStyleId = x;
        }

        float getRotaryDragSensitivity() const {
            return rotaryDragSensitivity;
        }

        void setRotaryDragSensitivity(const float x) {
            rotaryDragSensitivity = x;
        }

        void loadFromAPVTS();

        void saveToAPVTS();

    private:
        juce::AudioProcessorValueTreeState &state;
        std::atomic<float> fontSize{0};
        std::array<juce::Colour, colourNum> customColours;
        std::array<float, 2> wheelSensitivity{1.f, 0.12f};
        size_t rotaryStyleId{0};
        std::atomic<size_t> refreshRateId{2};
        float rotaryDragSensitivity{1.f};
        std::atomic<float> fftExtraTilt{0.f}, fftExtraSpeed{1.f};
        std::atomic<float> singleCurveThickness{1.f}, sumCurveThickness{1.f};

        float loadPara(const std::string &id) const {
            return state.getRawParameterValue(id)->load();
        }

        void savePara(const std::string &id, const float x) const {
            const auto para = state.getParameter(id);
            para->beginChangeGesture();
            para->setValueNotifyingHost(x);
            para->endChangeGesture();
        }
    };
}

#endif //ZL_INTERFACE_DEFINES_H
