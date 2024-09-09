// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlDSP {
    Controller::Controller() = default;

    void Controller::reset() {
        lrSplitter.reset();
        msSplitter.reset();
        lhSplitter.reset();
    }

    void Controller::prepare(const juce::dsp::ProcessSpec &spec) {
        lrSplitter.prepare(spec);
        msSplitter.prepare(spec);
        lhSplitter.prepare(spec);
        lhLinearSplitter.prepare(spec);
        tsSplitters[0].prepare(spec);
        tsSplitters[1].prepare(spec);

        currentMixSmooth.reset(spec.sampleRate, 0.01);

        meter1.prepare(spec);
        meter2.prepare(spec);

        internalBuffer.setSize(4, static_cast<int>(spec.maximumBlockSize));
    }

    void Controller::process(juce::AudioBuffer<double> &buffer) {
        internalBuffer.setSize(4, buffer.getNumSamples(), false, false, true);
        switch (splitType.load()) {
            case splitType::lright: {
                processLR(buffer);
                break;
            }
            case splitType::mside: {
                processMS(buffer);
                break;
            }
            case splitType::lhigh: {
                processLH(buffer);
                break;
            }
            case splitType::tsteady: {
                processTS(buffer);
                break;
            }
            case splitType::numSplit: {}
        }

        const juce::dsp::AudioBlock<double> block{buffer};
        const juce::dsp::AudioBlock<double> internalBlock{internalBuffer};

        switch (splitType.load()) {
            case splitType::lright:
            case splitType::mside:
            case splitType::lhigh: {
                currentMix = mix.load();
                currentMixSmooth.setTargetValue(currentMix);
                if (currentMixSmooth.isSmoothing()) {
                    for (size_t index = 0; index < block.getNumSamples(); ++index) {
                        const auto sampleMix = currentMixSmooth.getNextValue();
                        block.setSample(0, static_cast<int>(index),
                                        internalBlock.getSample(0, static_cast<int>(index)) * (1 - sampleMix)
                                        + internalBlock.getSample(2, static_cast<int>(index)) * sampleMix);
                        block.setSample(1, static_cast<int>(index),
                                        internalBlock.getSample(1, static_cast<int>(index)) * (1 - sampleMix)
                                        + internalBlock.getSample(3, static_cast<int>(index)) * sampleMix);
                    }
                } else {
                    block.getSubsetChannelBlock(0, 2).replaceWithProductOf(
                        internalBlock.getSubsetChannelBlock(0, 2), 1.0 - currentMix);
                    block.getSubsetChannelBlock(0, 2).addProductOf(
                        internalBlock.getSubsetChannelBlock(2, 2), currentMix);
                }
                if (block.getNumChannels() >= 4) {
                    block.getSubsetChannelBlock(2, 2).replaceWithSumOf(
                        internalBlock.getSubsetChannelBlock(0, 2), internalBlock.getSubsetChannelBlock(2, 2));
                    block.getSubsetChannelBlock(2, 2).subtract(block.getSubsetChannelBlock(0, 2));
                }
                break;
            }
            case splitType::tsteady: {
                block.getSubsetChannelBlock(0, 2).copyFrom(
                    internalBlock.getSubsetChannelBlock(0, 2));
                if (block.getNumChannels() >= 4) {
                    block.getSubsetChannelBlock(2, 2).copyFrom(
                        internalBlock.getSubsetChannelBlock(2, 2));
                }
                break;
            }
            case splitType::psteady: {
                buffer.clear();
            }
            case splitType::numSplit: {
                buffer.clear();
            }
        }

        meter1.process(block.getSubsetChannelBlock(0, 2));
        if (block.getNumChannels() >= 4) {
            meter2.process(block.getSubsetChannelBlock(2, 2));
        }
    }

    void Controller::processLR(juce::AudioBuffer<double> &buffer) {
        lrSplitter.split(buffer);
        const auto lBlock = juce::dsp::AudioBlock<double>(lrSplitter.getLBuffer());
        const auto rBlock = juce::dsp::AudioBlock<double>(lrSplitter.getRBuffer());
        const auto block = juce::dsp::AudioBlock<double>(internalBuffer);

        if (!swap.load()) {
            block.getSingleChannelBlock(0).copyFrom(lBlock);
            block.getSingleChannelBlock(1).fill(0.);
            block.getSingleChannelBlock(2).fill(0.);
            block.getSingleChannelBlock(3).copyFrom(rBlock);
        } else {
            block.getSingleChannelBlock(0).fill(0.);
            block.getSingleChannelBlock(1).copyFrom(rBlock);
            block.getSingleChannelBlock(2).copyFrom(lBlock);
            block.getSingleChannelBlock(3).fill(0.);
        }
    }

    void Controller::processMS(juce::AudioBuffer<double> &buffer) {
        msSplitter.split(buffer);
        const auto mBlock = juce::dsp::AudioBlock<double>(msSplitter.getMBuffer());
        const auto sBlock = juce::dsp::AudioBlock<double>(msSplitter.getSBuffer());
        const auto block = juce::dsp::AudioBlock<double>(internalBuffer);

        if (!swap.load()) {
            block.getSingleChannelBlock(0).copyFrom(mBlock);
            block.getSingleChannelBlock(1).copyFrom(mBlock);
            block.getSingleChannelBlock(2).copyFrom(sBlock);
            block.getSingleChannelBlock(3).replaceWithProductOf(sBlock, -1.0);
        } else {
            block.getSingleChannelBlock(0).copyFrom(sBlock);
            block.getSingleChannelBlock(1).replaceWithProductOf(sBlock, -1.0);
            block.getSingleChannelBlock(2).copyFrom(mBlock);
            block.getSingleChannelBlock(3).copyFrom(mBlock);
        }
    }

    void Controller::processLH(juce::AudioBuffer<double> &buffer) {
        juce::dsp::AudioBlock<double> lBlock, hBlock;
        if (lhFilterType.load() == lhFilterType::svf) {
            lhSplitter.split(buffer);
            lBlock = juce::dsp::AudioBlock<double>(lhSplitter.getLBuffer());
            hBlock = juce::dsp::AudioBlock<double>(lhSplitter.getHBuffer());
        } else {
            lhLinearSplitter.split(buffer);
            lBlock = juce::dsp::AudioBlock<double>(lhLinearSplitter.getLBuffer());
            hBlock = juce::dsp::AudioBlock<double>(lhLinearSplitter.getHBuffer());
        }
        const auto block = juce::dsp::AudioBlock<double>(internalBuffer);
        if (!swap.load()) {
            block.getSubsetChannelBlock(0, 2).copyFrom(lBlock);
            block.getSubsetChannelBlock(2, 2).copyFrom(hBlock);
        } else {
            block.getSubsetChannelBlock(0, 2).copyFrom(hBlock);
            block.getSubsetChannelBlock(2, 2).copyFrom(lBlock);
        }
    }

    void Controller::processTS(juce::AudioBuffer<double> &buffer) {
        juce::AudioBuffer<double> lBuffer{buffer.getArrayOfWritePointers(), 1, buffer.getNumSamples()};
        juce::AudioBuffer<double> rBuffer{buffer.getArrayOfWritePointers() + 1, 1, buffer.getNumSamples()};
        tsSplitters[0].split(lBuffer);
        tsSplitters[1].split(rBuffer);

        const auto block = juce::dsp::AudioBlock<double>(internalBuffer);

        const auto lTransientBlock = juce::dsp::AudioBlock<double>(tsSplitters[0].getTBuffer());
        const auto lSteadyBlock = juce::dsp::AudioBlock<double>(tsSplitters[0].getSBuffer());

        const auto rTransientBlock = juce::dsp::AudioBlock<double>(tsSplitters[1].getTBuffer());
        const auto rSteadyBlock = juce::dsp::AudioBlock<double>(tsSplitters[1].getSBuffer());

        if (!swap.load()) {
            block.getSingleChannelBlock(0).copyFrom(lTransientBlock);
            block.getSingleChannelBlock(1).copyFrom(rTransientBlock);
            block.getSingleChannelBlock(2).copyFrom(lSteadyBlock);
            block.getSingleChannelBlock(3).copyFrom(rSteadyBlock);
        } else {
            block.getSingleChannelBlock(0).copyFrom(lSteadyBlock);
            block.getSingleChannelBlock(1).copyFrom(rSteadyBlock);
            block.getSingleChannelBlock(2).copyFrom(lTransientBlock);
            block.getSingleChannelBlock(3).copyFrom(rTransientBlock);
        }
    }
} // zlDSP
