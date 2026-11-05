// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../fft/zldsp_fft_include.hpp"
#include "../../fft/zldsp_fft_window.hpp"
#include "../../vector/vector.hpp"

namespace zldsp::filter {
    template<typename FloatType, size_t DefaultFFTOrder = 10>
    class FIRBase {
    public:
        virtual ~FIRBase() = default;

        void prepare(const double sample_rate, const size_t num_channels) {
            if (sample_rate <= 50000) {
                setOrder(num_channels, DefaultFFTOrder);
            } else if (sample_rate <= 100000) {
                setOrder(num_channels, DefaultFFTOrder + 1);
            } else if (sample_rate <= 200000) {
                setOrder(num_channels, DefaultFFTOrder + 2);
            } else {
                setOrder(num_channels, DefaultFFTOrder + 3);
            }
            reset();
        }

        void reset() {
            pos_ = 0;
            count_ = 0;
            for (auto &fifo: input_fifo_) {
                fifo.resize(fft_size_);
                std::fill(fifo.begin(), fifo.end(), 0.f);
            }
            for (auto &fifo: output_fifo_) {
                fifo.resize(fft_size_);
                std::fill(fifo.begin(), fifo.end(), 0.f);
            }
            std::fill(fft_in_.begin(), fft_in_.end(), 0.f);
        }

        template<bool isBypassed = false>
        void process(std::span<FloatType *> buffer, const size_t num_samples) {
            process<isBypassed>(buffer, buffer, num_samples);
        }

        template<bool isBypassed = false>
        void process(std::span<FloatType* const> input, std::span<FloatType*> output, const size_t num_samples) {
            size_t offset = 0;
            while (offset < num_samples) {
                const auto chunk = std::min({num_samples - offset, hop_size_ - count_, fft_size_ - pos_});
                for (size_t chan = 0; chan < input.size(); ++chan) {
                    const auto* in = input[chan] + offset;
                    auto* out = output[chan] + offset;
                    auto* in_fifo = input_fifo_[chan].data() + pos_;
                    auto* out_fifo = output_fifo_[chan].data() + pos_;
                    for (size_t i = 0; i < chunk; ++i) {
                        in_fifo[i] = static_cast<float>(in[i]);
                        out[i] = static_cast<FloatType>(out_fifo[i]);
                        out_fifo[i] = 0.f;
                    }
                }

                pos_ += chunk;
                if (pos_ == fft_size_) {
                    pos_ = 0;
                }
                count_ += chunk;
                offset += chunk;
                if (count_ == hop_size_) {
                    count_ = 0;
                    processFrame<isBypassed>();
                }
            }
        }

        int getLatency() const { return latency_; }

    protected:
        std::unique_ptr<zldsp::fft::RFFT<float>> fft_;
        zldsp::vector::aligned_vector<float> window1_, window2_;

        size_t fft_order_ = DefaultFFTOrder;
        size_t fft_size_ = static_cast<size_t>(1) << fft_order_;
        size_t num_bins_ = fft_size_ / 2 + 1;
        size_t overlap_ = 4; // 75% overlap
        size_t hop_size_ = fft_size_ / overlap_;
        static constexpr float kWindowCorrection = 2.0f / 3.0f;
        static constexpr float kBypassCorrection = 1.0f / 4.0f;
        // counts up until the next hop.
        size_t count_ = 0;
        // write position in input FIFO and read position in output FIFO.
        size_t pos_ = 0;
        // circular buffers for incoming and outgoing audio data.
        std::vector<zldsp::vector::aligned_vector<float>> input_fifo_, output_fifo_;
        // real/imaginary planes including DC and Nyquist
        using Spectrum = std::array<zldsp::vector::aligned_vector<float>, 2>;
        zldsp::vector::aligned_vector<float> fft_in_;
        Spectrum fft_data_;

        std::array<float*, 2> spectrumPointers() {
            return {fft_data_[0].data(), fft_data_[1].data()};
        }

        int latency_{0};

        void setFFTOrder(const size_t num_channels, const size_t order) {
            fft_order_ = order;
            fft_size_ = static_cast<size_t>(1) << fft_order_;
            num_bins_ = fft_size_ / 2 + 1;
            hop_size_ = fft_size_ / overlap_;
            latency_ = static_cast<int>(fft_size_);

            if (fft_ == nullptr || fft_->get_order() != fft_order_) {
                fft_ = std::make_unique<zldsp::fft::RFFT<float>>(fft_order_);
            }

            window1_.resize(static_cast<size_t>(fft_size_));
            zldsp::fft::createPeriodicHanning<float>(window1_, 1.f / static_cast<float>(fft_size_));

            window2_.resize(static_cast<size_t>(fft_size_));
            // normalize the N/2 round-trip gain on synthesis
            zldsp::fft::createPeriodicHanning<float>(window2_, 2.f * kWindowCorrection);

            input_fifo_.resize(num_channels);
            output_fifo_.resize(num_channels);
            fft_in_.resize(fft_size_);
            for (auto& plane : fft_data_) {
                plane.resize(num_bins_);
            }
        }

        template<bool isBypassed = false>
        void processFrame() {
            for (size_t idx = 0; idx < input_fifo_.size(); ++idx) {
                if constexpr (!isBypassed) {
                    // copy circular buffer with analysis window
                    zldsp::vector::multiply(fft_in_.data(), input_fifo_[idx].data() + pos_,
                                           window1_.data(), fft_size_ - pos_);
                    if (pos_ > 0) {
                        zldsp::vector::multiply(fft_in_.data() + fft_size_ - pos_, input_fifo_[idx].data(),
                                               window1_.data() + fft_size_ - pos_, pos_);
                    }
                    fft_->forward(fft_in_.data(), spectrumPointers());
                    processSpectrum();
                    fft_->backward(spectrumPointers(), fft_in_.data());
                    zldsp::vector::multiply(fft_in_.data(), window2_.data(), fft_size_);
                } else {
                    zldsp::vector::multiply(fft_in_.data(), input_fifo_[idx].data() + pos_,
                                           kBypassCorrection, fft_size_ - pos_);
                    if (pos_ > 0) {
                        zldsp::vector::multiply(fft_in_.data() + fft_size_ - pos_, input_fifo_[idx].data(),
                                               kBypassCorrection, pos_);
                    }
                }
                zldsp::vector::add(output_fifo_[idx].data(), fft_in_.data() + fft_size_ - pos_, pos_);
                zldsp::vector::add(output_fifo_[idx].data() + pos_, fft_in_.data(), fft_size_ - pos_);
            }
        }

        virtual void setOrder(size_t num_channels, size_t order) = 0;

        virtual void processSpectrum() = 0;
    };
}
