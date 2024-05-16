// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#include "controller.hpp"

namespace zlDSP {
    Controller::Controller() {

    }

    void Controller::reset() {
        lrSplitter.reset();
        msSplitter.reset();
        lhSplitter.reset();
    }

    void Controller::prepare(const juce::dsp::ProcessSpec &spec) {
        lrSplitter.prepare(spec);
        msSplitter.prepare(spec);
        lhSplitter.prepare(spec);
    }

    void Controller::process(juce::AudioBuffer<double> &buffer) {
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
            case splitType::ttone: {
                buffer.clear();
                break;
            }
        }
    }

    void Controller::processLR(juce::AudioBuffer<double> &buffer) {
        lrSplitter.split(buffer);
        const auto currentMix = mix.load();
        const auto lBlock = juce::dsp::AudioBlock<double>(lrSplitter.getLBuffer());
        const auto rBlock = juce::dsp::AudioBlock<double>(lrSplitter.getRBuffer());
        const auto block = juce::dsp::AudioBlock<double>(buffer);

        const auto isSwap = swap.load();
        if (!isSwap) {
            block.getSingleChannelBlock(0).replaceWithProductOf(lBlock, 1.0 - currentMix);
            block.getSingleChannelBlock(1).replaceWithProductOf(rBlock, currentMix);
        } else {
            block.getSingleChannelBlock(0).replaceWithProductOf(lBlock, currentMix);
            block.getSingleChannelBlock(1).replaceWithProductOf(rBlock, 1.0 - currentMix);
        }
        if (buffer.getNumChannels() >= 4) {
            if (!isSwap) {
                block.getSingleChannelBlock(2).replaceWithProductOf(lBlock, currentMix);
                block.getSingleChannelBlock(3).replaceWithProductOf(rBlock, 1.0 - currentMix);
            } else {
                block.getSingleChannelBlock(2).replaceWithProductOf(lBlock, 1.0 - currentMix);
                block.getSingleChannelBlock(3).replaceWithProductOf(rBlock, currentMix);
            }
        }
    }

    void Controller::processMS(juce::AudioBuffer<double> &buffer) {
        msSplitter.split(buffer);
        const auto currentMix = mix.load();
        const auto mBlock = juce::dsp::AudioBlock<double>(msSplitter.getMBuffer());
        const auto sBlock = juce::dsp::AudioBlock<double>(msSplitter.getSBuffer());
        const auto block = juce::dsp::AudioBlock<double>(buffer);

        const auto isSwap = swap.load();
        if (!isSwap) {
            block.getSingleChannelBlock(0).replaceWithProductOf(mBlock, 1.0 - currentMix);
            block.getSingleChannelBlock(1).copyFrom(block.getSingleChannelBlock(0));
            block.getSingleChannelBlock(0).addProductOf(sBlock, -currentMix);
            block.getSingleChannelBlock(1).addProductOf(sBlock, currentMix);
        } else {
            block.getSingleChannelBlock(0).replaceWithProductOf(sBlock, 1.0 - currentMix);
            block.getSingleChannelBlock(1).replaceWithProductOf(sBlock, -(1.0 - currentMix));
            block.getSingleChannelBlock(0).addProductOf(mBlock, currentMix);
            block.getSingleChannelBlock(1).addProductOf(mBlock, currentMix);
        }
        if (buffer.getNumChannels() >= 4) {
            if (!isSwap) {
                block.getSingleChannelBlock(2).replaceWithProductOf(sBlock, 1.0 - currentMix);
                block.getSingleChannelBlock(3).replaceWithProductOf(sBlock, -(1.0 - currentMix));
                block.getSingleChannelBlock(2).addProductOf(mBlock, currentMix);
                block.getSingleChannelBlock(3).addProductOf(mBlock, currentMix);
            } else {
                block.getSingleChannelBlock(2).replaceWithProductOf(mBlock, 1.0 - currentMix);
                block.getSingleChannelBlock(3).copyFrom(block.getSingleChannelBlock(0));
                block.getSingleChannelBlock(2).addProductOf(sBlock, -currentMix);
                block.getSingleChannelBlock(3).addProductOf(sBlock, currentMix);
            }
        }
    }

    void Controller::processLH(juce::AudioBuffer<double> &buffer) {
        lhSplitter.split(buffer);
        const auto lBlock = juce::dsp::AudioBlock<double>(lhSplitter.getLBuffer());
        const auto hBlock = juce::dsp::AudioBlock<double>(lhSplitter.getHBuffer());
        const auto block = juce::dsp::AudioBlock<double>(buffer);
        const auto currentMix = mix.load();
        const auto isSwap = swap.load();
        if (!isSwap) {
            block.getSubsetChannelBlock(0, 2).replaceWithProductOf(lBlock, 1.0 - currentMix);
            block.getSubsetChannelBlock(0, 2).addProductOf(hBlock, currentMix);
        } else {
            block.getSubsetChannelBlock(0, 2).replaceWithProductOf(hBlock, 1.0 - currentMix);
            block.getSubsetChannelBlock(0, 2).addProductOf(lBlock, currentMix);
        }
        if (buffer.getNumChannels() >= 4) {
            if (!isSwap) {
                block.getSubsetChannelBlock(2, 2).replaceWithProductOf(hBlock, 1.0 - currentMix);
                block.getSubsetChannelBlock(2, 2).addProductOf(lBlock, currentMix);
            } else {
                block.getSubsetChannelBlock(2, 2).replaceWithProductOf(lBlock, 1.0 - currentMix);
                block.getSubsetChannelBlock(2, 2).addProductOf(hBlock, currentMix);
            }
        }
    }
} // zlDSP