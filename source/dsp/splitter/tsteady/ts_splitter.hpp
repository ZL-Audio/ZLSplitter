// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

// Fitzgerald, D. (2010). Harmonic/percussive separation using median filtering.

#ifndef TS_SPLITTER_HPP
#define TS_SPLITTER_HPP

#include <numbers>
#include <juce_dsp/juce_dsp.h>

#include "../../median_filter/median_filter.hpp"
#include "../../delay/delay.hpp"

namespace zlSplitter {
    /**
     * a splitter that splits the stereo audio signal input transient signal and steady signal
     * @tparam FloatType
     */
    template<typename FloatType>
    class TSSplitter {
    public:
        TSSplitter();

        void prepare(const juce::dsp::ProcessSpec &spec);

        /**
         * split the audio buffer into internal transient buffer and steady buffer
         * @param buffer
         */
        void split(juce::AudioBuffer<FloatType> &buffer);

        inline juce::AudioBuffer<FloatType> &getTBuffer() { return tBuffer; }

        inline juce::AudioBuffer<FloatType> &getSBuffer() { return sBuffer; }

        inline void setBalance(const float x) {
            balance.store(std::pow(16.f, x - 0.75f));
        }

        inline void setSeparation(const float x) {
            separation.store(std::exp(x * 4.f) - 1.f);
        }

        inline void setHold(const float x) {
            hold.store((32.f - std::pow(32.f, 1.f - x)) / 31.f * 0.75f + 0.24f);
        }

        inline void setSmooth(const float x) {
            smooth.store(x);
        }

        inline int getLatency() const { return latency.load(); }

    private:
        static constexpr size_t freqMedianWindowsSize = 5;
        static constexpr size_t freqHalfMedianWindowsSize = freqMedianWindowsSize / 2;
        static constexpr size_t timeMedianWindowsSize = 5;
        static constexpr size_t timeHalfMedianWindowsSize = timeMedianWindowsSize / 2;
        juce::AudioBuffer<FloatType> tBuffer, sBuffer;
        zlDelay::FIFODelay<FloatType> delay;
        // FFT parameters
        std::unique_ptr<juce::dsp::FFT> fft;
        std::unique_ptr<juce::dsp::WindowingFunction<float> > window;
        size_t fftOrder = 10;
        size_t fftSize = static_cast<size_t>(1) << fftOrder; // 1024 samples
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
        std::vector<float> transientFifo;
        // circular FFT working space which contains interleaved complex numbers.
        size_t fftDataPos = 0;
        std::array<std::vector<float>, timeHalfMedianWindowsSize + 1> fftDatas;
        std::vector<float> magnitude;
        // median calculators
        std::vector<zlMedianFilter::HeapFilter<float, timeMedianWindowsSize> > timeMedian{};
        zlMedianFilter::HeapFilter<float, freqMedianWindowsSize> freqMedian{};
        // portion holders
        std::vector<float> mask;
        // transient and steady spectrum
        std::vector<float> transientSpec;
        // seperation factor
        std::atomic<float> balance{5.f}, separation{1.f}, hold{0.9f}, smooth{.5f};
        float currentBalance{0.f}, currentSaperation{0.f}, currentHold{0.f}, currentSmooth{0.f};
        // latency
        std::atomic<int> latency{0};

        void setOrder(size_t order);

        void processFrame();

        void processSpectrum();

        float calculatePortion(float transientWeight, float steadyWeight);
    };
} // zlSplitter

#endif //TS_SPLITTER_HPP
