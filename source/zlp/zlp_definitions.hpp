// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace zlp {
    inline static constexpr int kVersionHint = 1;

    inline static constexpr size_t kBandNum = 8;

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRange(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid, const FloatType x_interval) {
        const FloatType rng1{std::log(x_mid / x_min) * FloatType(2)};
        const FloatType rng2{std::log(x_max / x_mid) * FloatType(2)};
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min, x_max,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5) ? std::exp(v * rng1) * x_min : std::exp((v - FloatType(.5)) * rng2) * x_mid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < x_mid ? std::log(v / x_min) / rng1 : FloatType(.5) + std::log(v / x_mid) / rng2;
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = x_min + x_interval * std::round((v - x_min) / x_interval);
                return x <= x_min ? x_min : (x >= x_max ? x_max : x);
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLogMidRangeShift(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid,
        const FloatType x_interval, const FloatType shift) {
        const auto range = getLogMidRange<FloatType>(x_min, x_max, x_mid, x_interval);
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min + shift, x_max + shift,
            [=](FloatType, FloatType, const FloatType v) {
                return range.convertFrom0to1(v) + shift;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return range.convertTo0to1(v - shift);
            },
            [=](FloatType, FloatType, const FloatType v) {
                return range.snapToLegalValue(v - shift) + shift;
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getSymmetricLogMidRangeShift(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid,
        const FloatType x_interval, const FloatType shift) {
        const auto range = getLogMidRangeShift<FloatType>(x_min, x_max, x_mid, x_interval, shift);
        auto result_range = juce::NormalisableRange<FloatType>{
            -(x_max + shift), x_max + shift,
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0.5)) {
                    return range.convertFrom0to1(v * FloatType(2) - FloatType(1));
                } else {
                    return -range.convertFrom0to1(FloatType(1) - v * FloatType(2));
                }
            },
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0)) {
                    return range.convertTo0to1(v) * FloatType(0.5) + FloatType(0.5);
                } else {
                    return FloatType(0.5) - range.convertTo0to1(-v) * FloatType(0.5);
                }
            },
            [=](FloatType, FloatType, const FloatType v) {
                if (v > FloatType(0)) {
                    return range.snapToLegalValue(v);
                } else {
                    return -range.snapToLegalValue(-v);
                }
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    template <typename FloatType>
    inline juce::NormalisableRange<FloatType> getLinearMidRange(
        const FloatType x_min, const FloatType x_max, const FloatType x_mid, const FloatType x_interval) {
        auto result_range = juce::NormalisableRange<FloatType>{
            x_min, x_max,
            [=](FloatType, FloatType, const FloatType v) {
                return v < FloatType(.5)
                    ? FloatType(2) * v * (x_mid - x_min) + x_min
                    : FloatType(2) * (v - FloatType(0.5)) * (x_max - x_mid) + x_mid;
            },
            [=](FloatType, FloatType, const FloatType v) {
                return v < x_mid
                    ? FloatType(.5) * (v - x_min) / (x_mid - x_min)
                    : FloatType(.5) + FloatType(.5) * (v - x_mid) / (x_max - x_mid);
            },
            [=](FloatType, FloatType, const FloatType v) {
                const FloatType x = x_min + x_interval * std::round((v - x_min) / x_interval);
                return x <= x_min ? x_min : (x >= x_max ? x_max : x);
            }
        };
        result_range.interval = x_interval;
        return result_range;
    }

    // float
    template <class T>
    class FloatParameters {
    public:
        static std::unique_ptr<juce::AudioParameterFloat> get(const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID, kVersionHint),
                                                               T::kName, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string& suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterFloat> get(const std::string& suffix, const bool meta,
                                                              const bool automate = true) {
            auto attributes = juce::AudioParameterFloatAttributes().withAutomatable(automate).withLabel(T::kName).
                withMeta(meta);
            return std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                               T::kName + suffix, T::kRange, T::kDefaultV, attributes);
        }

        inline static float convertTo01(const float x) {
            return T::kRange.convertTo0to1(x);
        }
    };

    // bool
    template <class T>
    class BoolParameters {
    public:
        static std::unique_ptr<juce::AudioParameterBool> get(bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID, kVersionHint),
                                                              T::kName, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string& suffix, bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        static std::unique_ptr<juce::AudioParameterBool> get(const std::string& suffix, const bool meta,
                                                             const bool automate = true) {
            auto attributes = juce::AudioParameterBoolAttributes().withAutomatable(automate).withLabel(T::kName).
                withMeta(meta);
            return std::make_unique<juce::AudioParameterBool>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                              T::kName + suffix, T::kDefaultV, attributes);
        }

        inline static float convertTo01(const bool x) {
            return x ? 1.f : 0.f;
        }
    };

    // choice
    template <class T>
    class ChoiceParameters {
    public:
        static std::unique_ptr<juce::AudioParameterChoice> get(const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID, kVersionHint),
                                                                T::kName, T::kChoices, T::kDefaultI, attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string& suffix, const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        static std::unique_ptr<juce::AudioParameterChoice> get(const std::string& suffix, const bool meta,
                                                               const bool automate = true) {
            auto attributes = juce::AudioParameterChoiceAttributes().withAutomatable(automate).withLabel(T::kName).
                withMeta(meta);
            return std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(T::kID + suffix, kVersionHint),
                                                                T::kName + suffix, T::kChoices, T::kDefaultI,
                                                                attributes);
        }

        inline static float convertTo01(const int x) {
            return static_cast<float>(x) / static_cast<float>(T::kChoices.size() - 1);
        }
    };

    class PSplitType : public ChoiceParameters<PSplitType> {
    public:
        auto static constexpr kID = "split_type";
        auto static constexpr kName = "Split Type";
        inline auto static const kChoices = juce::StringArray{
            "Left Right", "Mid Side", "Low High", "Transient Steady", "Peak Steady", "None"
        };
        int static constexpr kDefaultI = 5;

        enum SplitType {
            kLRight, kMSide, kLHigh, kTSteady, kPSteady, kNone
        };
    };

    class PMix : public FloatParameters<PMix> {
    public:
        auto static constexpr kID = "mix";
        auto static constexpr kName = "Mix";
        inline auto static const kRange = juce::NormalisableRange<float>(0, 100, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PSwap : public BoolParameters<PSwap> {
    public:
        auto static constexpr kID = "swap";
        auto static constexpr kName = "Swap";
        auto static constexpr kDefaultV = false;
    };

    class PBypass : public BoolParameters<PBypass> {
    public:
        auto static constexpr kID = "bypass";
        auto static constexpr kName = "Bypass";
        auto static constexpr kDefaultV = false;
    };

    class PLHFilterType : public ChoiceParameters<PLHFilterType> {
    public:
        auto static constexpr kID = "lh_filter_type";
        auto static constexpr kName = "LH Filter Type";
        inline auto static const kChoices = juce::StringArray{
            "SVF", "FIR"
        };

        int static constexpr kDefaultI = 0;

        enum FilterType {
            kSVF, kFIR
        };
    };

    class PLHSlope : public ChoiceParameters<PLHSlope> {
    public:
        auto static constexpr kID = "lh_slope";
        auto static constexpr kName = "LH Slope";
        inline auto static const kChoices = juce::StringArray{
            "12", "24", "48"
        };

        int static constexpr kDefaultI = 1;

        inline static constexpr std::array<size_t, 3> kOrders{1, 2, 4};
    };

    class PLHFreq : public FloatParameters<PLHFreq> {
    public:
        auto static constexpr kID = "lh_freq";
        auto static constexpr kName = "LH Freq";
        inline auto static const kRange = getLogMidRange(50.f, 18000.f, 1000.f, 0.1f);
        auto static constexpr kDefaultV = 1000.f;
    };

    class PTSBalance : public FloatParameters<PTSBalance> {
    public:
        auto static constexpr kID = "ts_balance";
        auto static constexpr kName = "TS Balance";
        inline auto static const kRange = juce::NormalisableRange<float>(-50.f, 50.f, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PTSSmooth : public FloatParameters<PTSSmooth> {
    public:
        auto static constexpr kID = "ts_smooth";
        auto static constexpr kName = "TS Smooth";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;
    };

    class PTSHold : public FloatParameters<PTSHold> {
    public:
        auto static constexpr kID = "ts_hold";
        auto static constexpr kName = "TS Hold";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;
    };

    class PTSStrength : public FloatParameters<PTSStrength> {
    public:
        auto static constexpr kID = "ts_strength";
        auto static constexpr kName = "TS Strength";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;
    };

    class PPSBalance : public FloatParameters<PPSBalance> {
    public:
        auto static constexpr kID = "ps_balance";
        auto static constexpr kName = "PS Balance";
        inline auto static const kRange = juce::NormalisableRange<float>(-50.f, 50.f, .1f);
        auto static constexpr kDefaultV = 0.f;
    };

    class PPSSmooth : public FloatParameters<PPSSmooth> {
    public:
        auto static constexpr kID = "ps_smooth";
        auto static constexpr kName = "PS Smooth";
        inline auto static const kRange = getLogMidRange(1.f, 100.f, 10.f, 0.1f);
        auto static constexpr kDefaultV = 10.f;
    };

    class PPSAttack : public FloatParameters<PPSAttack> {
    public:
        auto static constexpr kID = "ps_attack";
        auto static constexpr kName = "PS Attack";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;
    };

    class PPSHold : public FloatParameters<PPSHold> {
    public:
        auto static constexpr kID = "ps_hold";
        auto static constexpr kName = "PS Hold";
        inline auto static const kRange = juce::NormalisableRange<float>(0.f, 100.f, .1f);
        auto static constexpr kDefaultV = 50.f;
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add(PSplitType::get(), PMix::get(), PSwap::get(), PBypass::get(),
                   PLHFilterType::get(), PLHSlope::get(), PLHFreq::get(),
                   PTSBalance::get(), PTSStrength::get(), PTSHold::get(), PTSSmooth::get(),
                   PPSBalance::get(), PPSAttack::get(), PPSHold::get(), PPSSmooth::get());
        return layout;
    }
}
