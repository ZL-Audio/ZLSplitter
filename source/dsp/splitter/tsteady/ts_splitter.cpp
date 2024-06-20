// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

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
            setOrder(11);
        } else if (spec.sampleRate <= 100000) {
            setOrder(12);
        } else {
            setOrder(13);
        }
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::setOrder(const size_t order) {
        // set fft
        fftOrder = order;
        fftSize = 1 << fftOrder;
        numBins = fftSize / 2 + 1;
        hopSize = fftSize / overlap;
        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        window = std::make_unique<juce::dsp::WindowingFunction<float> >(
            fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false);
        // set fifo
        pos = 0;
        count = 0;
        inputFifo.resize(fftSize);
        std::fill(inputFifo.begin(), inputFifo.end(), 0.f);
        transientFifo.resize(fftSize);
        std::fill(transientFifo.begin(), transientFifo.end(), 0.f);
        steadyFifo.resize(fftSize);
        std::fill(steadyFifo.begin(), steadyFifo.end(), 0.f);
        // set fft data holder
        fftDataPos = 0;
        for (auto &data: fftDatas) {
            data.resize(2 * fftSize);
            std::fill(data.begin(), data.end(), 0.f);
        }
        magnitude.resize(numBins);
        transientSpec.resize(2 * fftSize);
        steadySpec.resize(2 * fftSize);
        // resize median calculators
        timeMedian.resize(numBins);
        mask.resize(numBins);
    }

    template<typename FloatType>
    void TSSplitter<FloatType>::split(juce::AudioBuffer<FloatType> &buffer) {
        tBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        sBuffer.setSize(1, buffer.getNumSamples(), true, false, true);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            inputFifo[pos] = static_cast<float>(buffer.getSample(0, i));
            tBuffer.setSample(0, i, static_cast<FloatType>(transientFifo[pos]));
            sBuffer.setSample(0, i, static_cast<FloatType>(steadyFifo[pos]));
            transientFifo[pos] = 0.f;
            steadyFifo[pos] = 0.f;

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
        fft->performRealOnlyInverseTransform(steadySpec.data());
        // apply the window again for resynthesis.
        window->multiplyWithWindowingTable(transientSpec.data(), fftSize);
        window->multiplyWithWindowingTable(steadySpec.data(), fftSize);
        // scale down the output samples because of the overlapping windows.
        for (size_t i = 0; i < fftSize; ++i) {
            transientSpec[i] *= windowCorrection;
            steadySpec[i] *= windowCorrection;
        }
        // add the ifft results to the output fifos
        for (size_t i = 0; i < pos; ++i) {
            transientFifo[i] += transientSpec[i + fftSize - pos];
            steadyFifo[i] += steadySpec[i + fftSize - pos];
        }
        for (size_t i = 0; i < fftSize - pos; ++i) {
            transientFifo[i + pos] += transientSpec[i];
            steadyFifo[i + pos] += steadySpec[i];
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
        for (size_t i = 0; i < halfMedianWindowsSize; ++i) {
            freqMedian.insert(0.f);
        }
        for (size_t i = 0; i < halfMedianWindowsSize; ++i) {
            freqMedian.insert(magnitude[i]);
        }
        for (size_t i = 0; i < numBins - halfMedianWindowsSize; ++i) {
            freqMedian.insert(magnitude[i + halfMedianWindowsSize]);
            timeMedian[i].insert(magnitude[i]);
            mask[i] = calculatePortion(freqMedian.getMedian(), timeMedian[i].getMedian());
        }
        for (size_t i = numBins - halfMedianWindowsSize; i < numBins; ++i) {
            freqMedian.insert(0.f);
            timeMedian[i].insert(magnitude[i]);
            mask[i] = calculatePortion(freqMedian.getMedian(), timeMedian[i].getMedian());
        }
        // retrieve fft data
        fftDataPos = (fftDataPos + 1) % (halfMedianWindowsSize + 1);
        // apply masks
        for (size_t i = 0; i < numBins; ++i) {
            const auto i1 = i * 2;
            const auto i2 = i1 + 1;
            transientSpec[i1] = fftDatas[fftDataPos][i1] * mask[i];
            transientSpec[i2] = fftDatas[fftDataPos][i2] * mask[i];
            steadySpec[i1] = fftDatas[fftDataPos][i1] * (1.f - mask[i]);
            steadySpec[i2] = fftDatas[fftDataPos][i2] * (1.f - mask[i]);
        }
    }

    template<typename FloatType>
    float TSSplitter<FloatType>::calculatePortion(const float transientWeight, const float steadyWeight) {
        return transientWeight / (transientWeight + steadyWeight);
    }

    template
    class TSSplitter<float>;

    template
    class TSSplitter<double>;
} // zlSplitter
