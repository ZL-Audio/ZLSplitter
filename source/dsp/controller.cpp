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
    }

    void Controller::prepare(const juce::dsp::ProcessSpec &spec) {
        lrSplitter.prepare(spec);
        msSplitter.prepare(spec);
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
                break;
            }
            case splitType::ttone: {
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
        std::array<juce::dsp::AudioBlock<double>, 4> blocks;
        for (size_t i = 0; i < 4; ++i) {
            blocks[i] = block.getSingleChannelBlock(i);
        }
        blocks[0].replaceWithProductOf(lBlock, 1.0 - currentMix);
        blocks[1].replaceWithProductOf(rBlock, currentMix);
        blocks[2].replaceWithProductOf(lBlock, currentMix);
        blocks[3].replaceWithProductOf(rBlock, 1.0 - currentMix);
    }

    void Controller::processMS(juce::AudioBuffer<double> &buffer) {
        msSplitter.split(buffer);
        const auto currentMix = mix.load();
        const auto mBlock = juce::dsp::AudioBlock<double>(msSplitter.getMBuffer());
        const auto sBlock = juce::dsp::AudioBlock<double>(msSplitter.getSBuffer());
        const auto block = juce::dsp::AudioBlock<double>(buffer);
        std::array<juce::dsp::AudioBlock<double>, 4> blocks;
        for (size_t i = 0; i < 4; ++i) {
            blocks[i] = block.getSingleChannelBlock(i);
        }
        blocks[0].replaceWithProductOf(mBlock, 1.0 - currentMix);
        blocks[1].copyFrom(blocks[0]);
        blocks[0].addProductOf(sBlock, -currentMix);
        blocks[1].addProductOf(sBlock, currentMix);

        blocks[2].replaceWithProductOf(sBlock, 1.0 - currentMix);
        blocks[3].replaceWithProductOf(sBlock, -(1.0 - currentMix));
        blocks[2].addProductOf(mBlock, currentMix);
        blocks[3].addProductOf(mBlock, currentMix);
    }
} // zlDSP