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

#include "../../chore/smoothed_value.hpp"
#include "tpt_filter.hpp"
#include "first_order_tpt_filter.hpp"

namespace zldsp::splitter {
    template<typename FloatType>
    class LHSplitter {
    public:
        LHSplitter() = default;

        void setMix(const FloatType mix) {
            mix_.setTarget(mix);
        }

        void setFreq(const double freq) {
            freq_.store(freq, std::memory_order::relaxed);
            to_update_freq_.store(true, std::memory_order::release);
        }

        void setOrder(const size_t order) {
            order_.store(order, std::memory_order::relaxed);
            to_update_order_.store(true, std::memory_order::release);
        }

        void prepare(const double sample_rate, const size_t num_channels) {
            mix_.prepare(sample_rate, 0.1);
            c_freq_.prepare(sample_rate, 0.125);

            for (auto &f: low1) {
                f.prepare(sample_rate, num_channels);
                f.setFreq(c_freq_.getCurrent());
            }

            for (auto &f: high1) {
                f.prepare(sample_rate, num_channels);
                f.setFreq(c_freq_.getCurrent());
            }

            for (auto &f: low2) {
                f.prepare(sample_rate, num_channels);
                f.setFreq(c_freq_.getCurrent());
            }

            for (auto &f: high2) {
                f.prepare(sample_rate, num_channels);
                f.setFreq(c_freq_.getCurrent());
            }
        }

        void prepareBuffer() {
            if (to_update_freq_.exchange(false, std::memory_order::acquire)) {
                c_freq_.setTarget(freq_.load(std::memory_order::relaxed));
            }
            if (to_update_order_.exchange(false, std::memory_order::acquire)) {
                c_order_ = order_.load(std::memory_order::relaxed);
                updateOrder(c_order_);
            }
        }

        void process(std::span<FloatType *> in_buffer,
                     std::span<FloatType *> low_buffer,
                     std::span<FloatType *> high_buffer,
                     const size_t num_samples) {
            if (c_order_ == 1) {
                processOrder1(in_buffer, low_buffer, high_buffer, num_samples);
            } else if (c_order_ == 2) {
                processOrder2(in_buffer, low_buffer, high_buffer, num_samples);
            } else if (c_order_ == 4) {
                processOrder4(in_buffer, low_buffer, high_buffer, num_samples);
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

    private:
        std::array<FirstOrderTPTFilter<FloatType>, 2> low1, high1;
        std::array<TPTFilter<FloatType>, 4> low2, high2;

        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> mix_{static_cast<FloatType>(0)};

        std::atomic<double> freq_{1000.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kFixMul> c_freq_{1000.0};
        std::atomic<bool> to_update_freq_{false};

        std::atomic<size_t> order_{2};
        size_t c_order_{2};
        std::atomic<bool> to_update_order_{true};

        static constexpr double order2q = 0.7071067811865476; // np.sqrt(2) / 2
        static constexpr double order4q1 = 0.541196100146197; // 1 / (2 * np.cos(np.pi / 8))
        static constexpr double order4q2 = 1.3065629648763764; // 1 / (2 * np.cos(np.pi / 8 * 3))

        void updateOrder(const size_t order) {
            if (order == 1) {
                for (size_t i = 0; i < 2; ++i) {
                    low1[i].reset();
                    high1[i].reset();
                    low1[i].setFreq(c_freq_.getCurrent());
                    high1[i].setFreq(c_freq_.getCurrent());
                }
            } else if (order == 2) {
                for (size_t i = 0; i < 2; ++i) {
                    low2[i].reset();
                    high2[i].reset();
                    low2[i].template setFreq<false>(c_freq_.getCurrent());
                    high2[i].template setFreq<false>(c_freq_.getCurrent());
                }
                low2[0].setQ(order2q);
                low2[1].setQ(order2q);
                high2[1].setQ(order2q);
            } else if (order == 4) {
                for (size_t i = 0; i < 4; ++i) {
                    low2[i].reset();
                    high2[i].reset();
                    low2[i].template setFreq<false>(c_freq_.getCurrent());
                    high2[i].template setFreq<false>(c_freq_.getCurrent());
                }
                low2[0].setQ(order4q1);
                low2[1].setQ(order4q1);
                high2[1].setQ(order4q1);
                low2[2].setQ(order4q2);
                high2[2].setQ(order4q2);
                low2[3].setQ(order4q2);
                high2[3].setQ(order4q2);
            }
        }

        void processOrder1(std::span<FloatType *> in_buffer,
                           std::span<FloatType *> low_buffer,
                           std::span<FloatType *> high_buffer,
                           const size_t num_samples) {
            if (c_freq_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto next_freq = c_freq_.getNext();
                    low1[0].setFreq(next_freq);
                    low1[1].setFreq(next_freq);
                    high1[1].setFreq(next_freq);
                    for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                        FloatType low_x, high_x;
                        low1[0].processSampleLowHigh(chan, in_buffer[chan][i], low_x, high_x);
                        low_buffer[chan][i] = low1[1].template processSample<
                            FirstOrderTPTFilter<FloatType>::TPTFilterType::kLowPass>(chan, low_x);
                        high_buffer[chan][i] = high1[1].template processSample<
                            FirstOrderTPTFilter<FloatType>::TPTFilterType::kHighPass>(chan, high_x);
                    }
                }
            } else {
                for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                    const auto in_chan = in_buffer[chan];
                    const auto low_chan = low_buffer[chan];
                    const auto high_chan = high_buffer[chan];
                    for (size_t i = 0; i < num_samples; ++i) {
                        FloatType low_x, high_x;
                        low1[0].processSampleLowHigh(chan, in_chan[i], low_x, high_x);
                        low_chan[i] = low1[1].template processSample<
                            FirstOrderTPTFilter<FloatType>::TPTFilterType::kLowPass>(chan, low_x);
                        high_chan[i] = high1[1].template processSample<
                            FirstOrderTPTFilter<FloatType>::TPTFilterType::kHighPass>(chan, high_x);
                    }
                }
            }
        }

        void processOrder2(std::span<FloatType *> in_buffer,
                           std::span<FloatType *> low_buffer,
                           std::span<FloatType *> high_buffer,
                           const size_t num_samples) {
            if (c_freq_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto next_freq = c_freq_.getNext();
                    low2[0].setFreq(next_freq);
                    low2[1].setFreq(next_freq);
                    high2[1].setFreq(next_freq);
                    for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                        FloatType low_x, high_x;
                        low2[0].processSampleLowHigh(chan, in_buffer[chan][i], low_x, high_x);
                        low_buffer[chan][i] = low2[1].template processSample<
                            TPTFilter<FloatType>::TPTFilterType::kLowPass>(chan, low_x);
                        high_buffer[chan][i] = high2[1].template processSample<
                            TPTFilter<FloatType>::TPTFilterType::kHighPass>(chan, high_x);
                    }
                }
            } else {
                for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                    const auto in_chan = in_buffer[chan];
                    const auto low_chan = low_buffer[chan];
                    const auto high_chan = high_buffer[chan];
                    for (size_t i = 0; i < num_samples; ++i) {
                        FloatType low_x, high_x;
                        low2[0].processSampleLowHigh(chan, in_chan[i], low_x, high_x);
                        low_chan[i] = low2[1].template processSample<
                            TPTFilter<FloatType>::TPTFilterType::kLowPass>(chan, low_x);
                        high_chan[i] = high2[1].template processSample<
                            TPTFilter<FloatType>::TPTFilterType::kHighPass>(chan, high_x);
                    }
                }
            }
        }

        void processOrder4(std::span<FloatType *> in_buffer,
                           std::span<FloatType *> low_buffer,
                           std::span<FloatType *> high_buffer,
                           const size_t num_samples) {
            if (c_freq_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    const auto next_freq = c_freq_.getNext();
                    for (size_t f_idx = 0; f_idx < 4; ++f_idx) {
                        low2[f_idx].setFreq(next_freq);
                    }
                    for (size_t f_idx = 1; f_idx < 4; ++f_idx) {
                        high2[f_idx].setFreq(next_freq);
                    }
                    for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                        FloatType low_x, high_x;
                        low2[0].processSampleLowHigh(chan, in_buffer[chan][i], low_x, high_x);
                        for (size_t f_idx = 1; f_idx < 4; ++f_idx) {
                            low_x = low2[f_idx].template processSample<
                                TPTFilter<FloatType>::TPTFilterType::kLowPass>(chan, low_x);
                            high_x = high2[f_idx].template processSample<
                                TPTFilter<FloatType>::TPTFilterType::kHighPass>(chan, high_x);
                        }
                        low_buffer[chan][i] = low_x;
                        high_buffer[chan][i] = high_x;
                    }
                }
            } else {
                for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                    const auto in_chan = in_buffer[chan];
                    const auto low_chan = low_buffer[chan];
                    const auto high_chan = high_buffer[chan];
                    for (size_t i = 0; i < num_samples; ++i) {
                        FloatType low_x, high_x;
                        low2[0].processSampleLowHigh(chan, in_chan[i], low_x, high_x);
                        for (size_t f_idx = 1; f_idx < 4; ++f_idx) {
                            low_x = low2[f_idx].template processSample<
                                TPTFilter<FloatType>::TPTFilterType::kLowPass>(chan, low_x);
                            high_x = high2[f_idx].template processSample<
                                TPTFilter<FloatType>::TPTFilterType::kHighPass>(chan, high_x);
                        }
                        low_chan[i] = low_x;
                        high_chan[i] = high_x;
                    }
                }
            }
        }
    };
}
