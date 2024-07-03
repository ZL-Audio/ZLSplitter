// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

// Fitzgerald, D. (2010). Harmonic/percussive separation using median filtering.

#ifndef TS_SPLITTER_HPP
#define TS_SPLITTER_HPP

#include <numbers>
#include <juce_dsp/juce_dsp.h>

#include "../../median_filter/median_filter.hpp"

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input transient signal and steady signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class TSSplitter {
    public:
        TSSplitter();

        void reset();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal transient buffer and steady buffer
         * @param buffer
         */
        void split(juce::AudioBuffer<FloatType> &buffer);

        inline juce::AudioBuffer<FloatType> &getTBuffer() { return tBuffer; }

        inline juce::AudioBuffer<FloatType> &getSBuffer() { return sBuffer; }

        inline void setFactor(const float x) { factor.store(x); }

        inline int getLatency() const { return latency.load(); }

    private:
        static constexpr size_t freqMedianWindowsSize = 11;
        static constexpr size_t freqHalfMedianWindowsSize = freqMedianWindowsSize / 2;
        static constexpr size_t timeMedianWindowsSize = 11;
        static constexpr size_t timeHalfMedianWindowsSize = timeMedianWindowsSize / 2;
        juce::AudioBuffer<FloatType> tBuffer, sBuffer;
        // FFT parameters
        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        size_t fftOrder = 10;
        size_t fftSize = 1 << fftOrder; // 1024 samples
        size_t numBins = fftSize / 2 + 1; // 513 bins
        size_t overlap = 4; // 75% overlap
        size_t hopSize = fftSize / overlap; // 256 samples
        // gain correction for using Hann window with 75% overlap.
        static constexpr float windowCorrection = 2.0f / 3.0f;
        // counts up until the next hop.
        size_t count = 0;
        // write position in input FIFO and read position in output FIFO.
        size_t pos = 0;
        // circular buffers for incoming and outgoing audio data.
        std::vector<float> inputFifo;
        std::vector<float> transientFifo, steadyFifo;
        // circular FFT working space which contains interleaved complex numbers.
        size_t fftDataPos = 0;
        std::array<std::vector<float>, timeHalfMedianWindowsSize + 1> fftDatas;
        std::vector<float> magnitude;
        // median calculators
        std::vector<zlMedianFilter::HeapFilter<float, timeMedianWindowsSize> > timeMedian{};
        zlMedianFilter::HeapFilter<float, freqMedianWindowsSize> freqMedian{};
        // portion holders
        std::vector<float> mask, previousMask;
        // transient and steady spectrum
        std::vector<float> transientSpec, steadySpec;
        // seperation factor
        std::atomic<float> factor{.5f};
        float currentFactor1{.5f}, currentFactor2{.5f};
        // latency
        std::atomic<int> latency{0};

        void setOrder(size_t order);

        void processFrame();

        void processSpectrum();

        float calculatePortion(float transientWeight, float steadyWeight);
    };
} // zlSplitter

#endif //TS_SPLITTER_HPP
