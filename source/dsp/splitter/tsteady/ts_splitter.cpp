// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "ts_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    TSSplitter<FloatType>::TSSplitter() {
        setOrder(11);
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        tBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        sBuffer.setSize(1, static_cast<int>(spec.maximumBlockSize));
        if (spec.sampleRate <= 50000) {
            setOrder(10);
        } else if (spec.sampleRate <= 100000) {
            setOrder(11);
        } else {
            setOrder(12);
        }
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::setOrder(const size_t order) {
        // set fft
        fftOrder = order;
        fftSize = static_cast<size_t>(1) << fftOrder;
        numBins = fftSize / 2 + 1;
        hopSize = fftSize / overlap;
        const auto d = static_cast<int>(fftSize + hopSize * timeHalfMedianWindowsSize);
        latency.store(d);
        delay.setMaximumDelayInSamples(d);
        delay.setDelay(d);

        fft = std::make_unique<juce::dsp::FFT>(static_cast<int>(fftOrder));
        window = std::make_unique<juce::dsp::WindowingFunction<float> >(
            fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false);
        // set fifo
        pos = 0;
        count = 0;
        inputFifo.resize(fftSize);
        std::fill(inputFifo.begin(), inputFifo.end(), 0.f);
        transientFifo.resize(fftSize);
        std::fill(transientFifo.begin(), transientFifo.end(), 0.f);
        // set fft data holder
        fftDataPos = 0;
        for (auto &data: fftDatas) {
            data.resize(2 * fftSize);
            std::fill(data.begin(), data.end(), 0.f);
        }
        magnitude.resize(numBins);
        transientSpec.resize(2 * fftSize);
        // resize median calculators
        timeMedian.resize(numBins);
        mask.resize(numBins);
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        tBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        sBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        auto readPointer = buffer.getReadPointer(0);
        auto tPointer = tBuffer.getWritePointer(0);
        auto sPointer = sBuffer.getWritePointer(0);
        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            inputFifo[pos] = static_cast<float>(readPointer[i]);
            tPointer[i] = static_cast<FloatType>(transientFifo[pos]);
            transientFifo[pos] = 0.f;

            pos += 1;
            if (pos == fftSize) {
                pos = 0;
            }

            count += 1;
            if (count == hopSize) {
                count = 0;
                processFrame();
            }
        }

        for (size_t i = 0; i < static_cast<size_t>(buffer.getNumSamples()); ++i) {
            sPointer[i] = delay.push(readPointer[i]) - tPointer[i];
        }
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::processFrame() {
        // copy data from input fifo to fft working place
        const float *inputPtr = inputFifo.data();
        float *fftPtr = fftDatas[fftDataPos].data();
        std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
        if (pos > 0) {
            std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
        }
        // apply the window to avoid spectral leakage
        window->multiplyWithWindowingTable(fftPtr, fftSize);
        // perform the forward fft
        fft->performRealOnlyForwardTransform(fftPtr, true);
        // split
        processSpectrum();
        // perform the inverse fft
        fft->performRealOnlyInverseTransform(transientSpec.data());
        // apply the window again for resynthesis.
        window->multiplyWithWindowingTable(transientSpec.data(), fftSize);
        // scale down the output samples because of the overlapping windows.
        for (size_t i = 0; i < fftSize; ++i) {
            transientSpec[i] *= windowCorrection;
        }
        // add the ifft results to the output fifos
        for (size_t i = 0; i < pos; ++i) {
            transientFifo[i] += transientSpec[i + fftSize - pos];
        }
        for (size_t i = 0; i < fftSize - pos; ++i) {
            transientFifo[i + pos] += transientSpec[i];
        }
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::processSpectrum() {
        // calculate magnitude
        for (size_t i = 0; i < numBins; ++i) {
            const auto re = fftDatas[fftDataPos][2 * i];
            const auto im = fftDatas[fftDataPos][2 * i + 1];
            magnitude[i] = std::sqrt(re * re + im * im);
        }
        // calculate median and masks
        freqMedian.clear();
        for (size_t i = 0; i < freqHalfMedianWindowsSize; ++i) {
            freqMedian.insert(magnitude[0]);
        }
        for (size_t i = 0; i < freqHalfMedianWindowsSize; ++i) {
            freqMedian.insert(magnitude[i]);
        }
        currentBalance = balance.load();
        currentSeperation = seperation.load();
        currentHold = hold.load();
        for (size_t i = 0; i < numBins - freqHalfMedianWindowsSize; ++i) {
            freqMedian.insert(magnitude[i + freqHalfMedianWindowsSize]);
            timeMedian[i].insert(magnitude[i]);
            const auto currentMask = calculatePortion(freqMedian.getMedian(), timeMedian[i].getMedian());
            mask[i] = std::max(mask[i] * currentHold, currentMask);
        }
        for (size_t i = numBins - freqHalfMedianWindowsSize; i < numBins; ++i) {
            freqMedian.insert(magnitude[numBins - 1]);
            timeMedian[i].insert(magnitude[i]);
            const auto currentMask = calculatePortion(freqMedian.getMedian(), timeMedian[i].getMedian());
            mask[i] = std::max(mask[i] * currentHold, currentMask);
        }
        // retrieve fft data
        currentSmooth = smooth.load();
        auto allMask = std::reduce(mask.begin(), mask.end()) / static_cast<float>(mask.size());
        allMask = juce::jlimit(-.5f, .5f, (allMask - 0.5f) * std::sqrt(currentSeperation)) + .5f;
        fftDataPos = (fftDataPos + 1) % (timeHalfMedianWindowsSize + 1);
        // apply masks
        for (size_t i = 0; i < numBins; ++i) {
            const auto i1 = i * 2;
            const auto i2 = i1 + 1;
            const auto binMask = allMask * currentSmooth + mask[i] * (1.f - currentSmooth);
            transientSpec[i1] = fftDatas[fftDataPos][i1] * binMask;
            transientSpec[i2] = fftDatas[fftDataPos][i2] * binMask;
        }
    }

    template<typename FloatType>
    float TSSplitter<FloatType>::calculatePortion(const float transientWeight, const float steadyWeight) {
        const auto t = transientWeight * currentBalance;
        const auto s = steadyWeight;
        const auto tt = t * t;
        const auto ss = s * s;
        const auto p = tt / std::max(tt + ss, 0.00000001f);
        return juce::jlimit(-.5f, .5f, (p - 0.5f) * currentSeperation) + .5f;
    }

    template
    class TSSplitter<float>;

    template
    class TSSplitter<double>;
} // zlSplitter
