// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <atomic>
#include <numbers>

#include "../../chore/smoothed_value.hpp"
#include "../../delay/integer_delay.hpp"
#include "../../filter/filter.hpp"

namespace zldsp::splitter {
    template<typename FloatType>
    class LHFIRSplitter {
    public:
        LHFIRSplitter() {
        }

        void setMix(const FloatType mix) {
            mix_.setTarget(mix);
        }

        void setFreq(const double freq) {
            freq_.store(freq, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void setOrder(const size_t order) {
            order_.store(order, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        void prepare(const double sample_rate,
                     const size_t num_channels,
                     const size_t max_num_samples) {
            mix_.prepare(sample_rate, 0.1);

            if (sample_rate <= 50000.0) {
                extra_stage_ = 0;
            } else if (sample_rate <= 100000.0) {
                extra_stage_ = 1;
            } else if (sample_rate <= 200000.0) {
                extra_stage_ = 2;
            } else {
                extra_stage_ = 3;
            }
            sample_rate_ = sample_rate;

            first_order_filter_.setNumStage(kFirstOrderNumStage + extra_stage_);
            first_order_filter_.prepare(num_channels);
            filter_[0].setNumStage(kSecondOrderNumStage + extra_stage_);
            filter_[0].prepare(max_num_samples, num_channels);
            filter_[1].setNumStage(kSecondOrderNumStage + extra_stage_);
            filter_[1].prepare(max_num_samples, num_channels);
            forward_filter_[0].prepare(num_channels);
            forward_filter_[1].prepare(num_channels);

            const auto max_delay = static_cast<FloatType>((1 << (kSecondOrderNumStage + extra_stage_ + 2)) + 2);

            delay_.prepare(sample_rate, max_num_samples, num_channels, max_delay / static_cast<FloatType>(sample_rate));

            to_update_.store(true, std::memory_order::release);
        }

        void prepareBuffer() {
            if (to_update_.exchange(false, std::memory_order::acquire)) {
                const auto new_order = order_.load(std::memory_order::relaxed);
                if (c_order_ != new_order) {
                    c_order_ = new_order;
                    updateOrder(c_order_);
                }
                updateFreq(freq_.load(std::memory_order::relaxed));
            }
        }

        void process(std::span<FloatType *> in_buffer,
                     std::span<FloatType *> low_buffer,
                     std::span<FloatType *> high_buffer,
                     const size_t num_samples) {
            zldsp::vector::copy(low_buffer, in_buffer, num_samples);
            zldsp::vector::copy(high_buffer, in_buffer, num_samples);

            switch (c_order_) {
                case 1: {
                    first_order_filter_.process(low_buffer, num_samples);
                    forward_filter_[0].process(low_buffer, num_samples);
                    break;
                }
                case 2: {
                    filter_[0].process(low_buffer, num_samples);
                    forward_filter_[0].process(low_buffer, num_samples);
                    break;
                }
                case 4: {
                    filter_[0].process(low_buffer, num_samples);
                    forward_filter_[0].process(low_buffer, num_samples);
                    filter_[1].process(low_buffer, num_samples);
                    forward_filter_[1].process(low_buffer, num_samples);
                    break;
                }
                default: {}
            }

            delay_.process(high_buffer, num_samples);
            for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                auto low_v = kfr::make_univector(low_buffer[chan], num_samples);
                auto high_v = kfr::make_univector(high_buffer[chan], num_samples);
                high_v = high_v - low_v;
            }

            if (mix_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto mix = mix_.getNext();
                    for (size_t chan = 0; chan < low_buffer.size(); ++chan) {
                        const auto low = low_buffer[chan][i];
                        const auto high = high_buffer[chan][i];
                        const auto diff = high - low;
                        low_buffer[chan][i] = low + mix * diff;
                        high_buffer[chan][i] = high - mix * diff;
                    }
                }
            } else if (mix_.getCurrent() > static_cast<FloatType>(1e-6)) {
                const auto mix = mix_.getCurrent();
                for (size_t chan = 0; chan < low_buffer.size(); ++chan) {
                    const auto low_chan = low_buffer[chan];
                    const auto high_chan = high_buffer[chan];
                    for (size_t i = 0; i < num_samples; ++i) {
                        const auto low = low_chan[i];
                        const auto high = high_chan[i];
                        const auto diff = high - low;
                        low_chan[i] = low + mix * diff;
                        high_chan[i] = high - mix * diff;
                    }
                }
            }
        }

        int getLatency() const {
            switch (c_order_) {
                case 1: return static_cast<int>(1 << (kFirstOrderNumStage + extra_stage_ + 1));
                case 2: return static_cast<int>(1 << (kSecondOrderNumStage + extra_stage_ + 1)) + 1;
                case 4: return static_cast<int>(1 << (kSecondOrderNumStage + extra_stage_ + 2)) + 2;
                default: return 0;
            }
        }

    private:
        inline static constexpr size_t kFirstOrderNumStage = 9;
        inline static constexpr size_t kSecondOrderNumStage = 11;
        size_t extra_stage_{0};

        zldsp::delay::IntegerDelay<FloatType> delay_;

        zldsp::filter::ReverseFirstOrderIIRBase<FloatType> first_order_filter_{kFirstOrderNumStage};

        std::array<zldsp::filter::ReverseIIRBase<FloatType>, 2> filter_ = {
            zldsp::filter::ReverseIIRBase<FloatType>(kSecondOrderNumStage),
            zldsp::filter::ReverseIIRBase<FloatType>(kSecondOrderNumStage)
        };

        std::array<zldsp::filter::IIRBase<FloatType>, 2> forward_filter_;

        double sample_rate_{48000.0};

        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> mix_{static_cast<FloatType>(0)};

        std::atomic<double> freq_{1000.0};

        std::atomic<size_t> order_{2};
        size_t c_order_{0};
        std::atomic<bool> to_update_{true};

        static constexpr double order2q = 0.7071067811865476; // np.sqrt(2) / 2
        static constexpr double order4q1 = 0.541196100146197; // 1 / (2 * np.cos(np.pi / 8))
        static constexpr double order4q2 = 1.3065629648763764; // 1 / (2 * np.cos(np.pi / 8 * 3))

        void updateOrder(const size_t order) {
            delay_.setDelayInSamples(getLatency());

            switch (order) {
                case 1: {
                    first_order_filter_.reset();
                    forward_filter_[0].reset();
                    break;
                }
                case 2: {
                    filter_[0].reset();
                    forward_filter_[0].reset();
                    break;
                }
                case 4: {
                    filter_[0].reset();
                    forward_filter_[0].reset();
                    filter_[1].reset();
                    forward_filter_[1].reset();
                    break;
                }
                default: {
                }
            }
        }

        void updateFreq(const double freq) {
            switch (c_order_) {
                case 1: {
                    const auto coeff = zldsp::filter::MartinCoeff::get1LowPass(
                        2 * std::numbers::pi * freq / sample_rate_);
                    first_order_filter_.updateFromBiquad(coeff);
                    forward_filter_[0].updateFromBiquad({coeff[0], coeff[1], 0.0, coeff[2], coeff[3], 0.0});
                    break;
                }
                case 2: {
                    const auto coeff = zldsp::filter::MartinCoeff::get2LowPass(
                        2 * std::numbers::pi * freq / sample_rate_, order2q);
                    filter_[0].updateFromBiquad(coeff);
                    forward_filter_[0].updateFromBiquad(coeff);
                    break;
                }
                case 4: {
                    {
                        const auto coeff = zldsp::filter::MartinCoeff::get2LowPass(
                            2 * std::numbers::pi * freq / sample_rate_, order4q1);
                        filter_[0].updateFromBiquad(coeff);
                        forward_filter_[0].updateFromBiquad(coeff);
                    } {
                        const auto coeff = zldsp::filter::MartinCoeff::get2LowPass(
                            2 * std::numbers::pi * freq / sample_rate_, order4q2);
                        filter_[1].updateFromBiquad(coeff);
                        forward_filter_[1].updateFromBiquad(coeff);
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    };
}
