// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLSPLIT_DSP_DEFINITIONS_HPP
#define ZLSPLIT_DSP_DEFINITIONS_HPP

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlDSP {
    inline auto static constexpr versionHint = 1;

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

        static std::unique_ptr<juce::AudioParameterBool> get(bool meta, const std::string &suffix = "",
                                                             bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::name).
                    withMeta(meta);
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
            return static_cast<float>(x) / static_cast<float>(T::choices.size());
        }
    };

    class splitType : public ChoiceParameters<splitType> {
    public:
        auto static constexpr ID = "split_type";
        auto static constexpr name = "";
        inline auto static const choices = juce::StringArray{
            "Left Right", "Mid Side", "Low High", "Transient Steady"
        };
        int static constexpr defaultI = 0;

        enum stype {
            lright, mside, lhigh, tsteady
        };
    };

    class mix : public FloatParameters<mix> {
    public:
        auto static constexpr ID = "mix";
        auto static constexpr name = "Mix";
        inline auto static const range = juce::NormalisableRange<float>(0, 100, .1f);
        auto static constexpr defaultV = 0.f;
    };

    class swap : public BoolParameters<swap> {
    public:
        auto static constexpr ID = "swap";
        auto static constexpr name = "Swap";
        auto static constexpr defaultV = false;
    };

    class lhSlope : public ChoiceParameters<lhSlope> {
    public:
        auto static constexpr ID = "lh_slope";
        auto static constexpr name = "LH Slope";
        inline auto static const choices = juce::StringArray{
            "12 dB/oct", "24 dB/oct", "48 dB/oct"
        };

        int static constexpr defaultI = 1;

        inline static constexpr std::array<size_t, 3> orders{1, 2, 4};
    };

    class lhFreq : public FloatParameters<lhFreq> {
    public:
        auto static constexpr ID = "lh_freq";
        auto static constexpr name = "LH Freq";
        inline auto static const range = juce::NormalisableRange<float>(10, 20000, .1f, 0.23064293761596813f);
        auto static constexpr defaultV = 1000.f;
    };

    class tsBalance : public FloatParameters<tsBalance> {
    public:
        auto static constexpr ID = "ts_balance";
        auto static constexpr name = "Balance";
        inline auto static const range = juce::NormalisableRange<float>(-50.f, 50.f, .1f);
        auto static constexpr defaultV = 0.f;

        inline static float formatV(const float x) {
            return x / 100.f + .5f;
        }
    };

    class tsHold : public FloatParameters<tsHold> {
    public:
        auto static constexpr ID = "ts_hold";
        auto static constexpr name = "Hold";
        inline auto static const range = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr defaultV = 50.f;

        inline static float formatV(const float x) {
            return x / 100.f;
        }
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(splitType::get(), mix::get(), swap::get(),
                   lhSlope::get(), lhFreq::get(),
                   tsBalance::get(), tsHold::get());
        return layout;
    }
}

#endif //ZLSPLIT_DSP_DEFINITIONS_HPP
