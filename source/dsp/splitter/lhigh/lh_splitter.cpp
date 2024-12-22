// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "lh_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    LHSplitter<FloatType>::LHSplitter() {
        for (size_t i = 0; i < 2; ++i) {
            low1[i].setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
            high1[i].setType(juce::dsp::FirstOrderTPTFilterType::highpass);
        }

        for (auto &f: low2) {
            f.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        }

        for (auto &f: high2) {
            f.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        }
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::reset() {
        toReset.store(true);
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        for (size_t i = 0; i < 2; ++i) {
            low1[i].prepare(spec);
            high1[i].prepare(spec);
        }

        for (auto &f: low2) {
            f.prepare(spec);
        }

        for (auto &f: high2) {
            f.prepare(spec);
        }

        lBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
        hBuffer.setSize(2, static_cast<int>(spec.maximumBlockSize));
        toUpdate.store(true);
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        split(juce::dsp::AudioBlock<FloatType>(buffer));
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::split(juce::dsp::AudioBlock<FloatType> block) {
        auto subBlock = block.getSubsetChannelBlock(0, 2);
        if (toUpdateOrder.exchange(false)) {
            updateOrder();
        }
        if (toReset.exchange(false)) {
            for (size_t i = 0; i < 2; ++i) {
                low1[i].reset();
                high1[i].reset();
            }

            for (auto &f: low2) {
                f.reset();
            }

            for (auto &f: high2) {
                f.reset();
            }
        }
        lBuffer.setSize(
            static_cast<int>(subBlock.getNumChannels()),
            static_cast<int>(subBlock.getNumSamples()),
            false, false, true);
        hBuffer.setSize(
            static_cast<int>(subBlock.getNumChannels()),
            static_cast<int>(subBlock.getNumSamples()),
            false, false, true);
        juce::dsp::AudioBlock<FloatType> lBlock{lBuffer};
        juce::dsp::AudioBlock<FloatType> hBlock{hBuffer};
        lBlock.copyFrom(subBlock);
        hBlock.copyFrom(subBlock);


        switch (currentOrder) {
            case 1: {
                for (size_t index = 0; index < lBlock.getNumSamples(); ++index) {
                    if (toUpdate.exchange(false)) {
                        update();
                    }
                    for (size_t channel = 0; channel < lBlock.getNumChannels(); ++channel) {
                        auto lWriter = lBlock.getChannelPointer(channel);
                        auto rWriter = hBlock.getChannelPointer(channel);
                        for (size_t i = 0; i < 2; ++i) {
                            lWriter[index] = low1[i].processSample(static_cast<int>(channel), lWriter[index]);
                            rWriter[index] = high1[i].processSample(static_cast<int>(channel), rWriter[index]);
                        }
                    }
                }
                for (size_t channel = 0; channel < hBlock.getNumChannels(); ++channel) {
                    juce::FloatVectorOperations::multiply(hBlock.getChannelPointer(channel),
                                                          static_cast<FloatType>(-1.0),
                                                          static_cast<int>(block.getNumSamples()));
                }
                break;
            }
            case 2: {
                for (size_t index = 0; index < lBlock.getNumSamples(); ++index) {
                    if (toUpdate.exchange(false)) {
                        update();
                    }
                    for (size_t channel = 0; channel < lBlock.getNumChannels(); ++channel) {
                        auto lWriter = lBlock.getChannelPointer(channel);
                        auto rWriter = hBlock.getChannelPointer(channel);
                        for (size_t i = 0; i < 2; ++i) {
                            lWriter[index] = low2[i].processSample(static_cast<int>(channel), lWriter[index]);
                            rWriter[index] = high2[i].processSample(static_cast<int>(channel), rWriter[index]);
                        }
                    }
                }
                break;
            }
            case 4: {
                for (size_t index = 0; index < lBlock.getNumSamples(); ++index) {
                    if (toUpdate.exchange(false)) {
                        update();
                    }
                    for (size_t channel = 0; channel < lBlock.getNumChannels(); ++channel) {
                        auto lWriter = lBlock.getChannelPointer(channel);
                        auto rWriter = hBlock.getChannelPointer(channel);
                        for (size_t i = 0; i < 4; ++i) {
                            lWriter[index] = low2[i].processSample(static_cast<int>(channel), lWriter[index]);
                            rWriter[index] = high2[i].processSample(static_cast<int>(channel), rWriter[index]);
                        }
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::setFreq(const FloatType x) {
        freq.store(x);
        toUpdate.store(true);
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::setOrder(const size_t x) {
        order.store(x);
        toUpdateOrder.store(true);
        toUpdate.store(true);
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::update() {
        currentFreq = freq.load();
        switch (currentOrder) {
            case 1: {
                for (size_t i = 0; i < 2; ++i) {
                    low1[i].setCutoffFrequency(currentFreq);
                    high1[i].setCutoffFrequency(currentFreq);
                }
            }
            case 2: {
                for (size_t i = 0; i < 2; ++i) {
                    low2[i].setCutoffFrequency(currentFreq);
                    high2[i].setCutoffFrequency(currentFreq);
                }
                break;
            }
            case 4: {
                for (size_t i = 0; i < 4; ++i) {
                    low2[i].setCutoffFrequency(currentFreq);
                    high2[i].setCutoffFrequency(currentFreq);
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    template<typename FloatType>
    void LHSplitter<FloatType>::updateOrder() {
        if (currentOrder != order.load()) {
            currentOrder = order.load();
            switch (currentOrder) {
                case 2: {
                    for (size_t i = 0; i < 2; ++i) {
                        low2[i].setResonance(static_cast<FloatType>(order2q));
                        high2[i].setResonance(static_cast<FloatType>(order2q));
                    }
                    break;
                }
                case 4: {
                    for (size_t i = 0; i < 2; ++i) {
                        low2[i].setResonance(static_cast<FloatType>(order4q1));
                        high2[i].setResonance(static_cast<FloatType>(order4q1));
                    }
                    for (size_t i = 2; i < 4; ++i) {
                        low2[i].setResonance(static_cast<FloatType>(order4q2));
                        high2[i].setResonance(static_cast<FloatType>(order4q2));
                    }
                    break;
                }
                default: {
                }
            }
        }
    }

    template
    class LHSplitter<float>;

    template
    class LHSplitter<double>;
} // zlSplitter
