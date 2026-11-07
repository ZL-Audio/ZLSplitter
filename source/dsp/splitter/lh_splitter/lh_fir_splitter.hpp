// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <span>
#include <utility>

#include "../../chore/smoothed_value.hpp"
#include "../../delay/integer_delay.hpp"
#include "../../filter/filter_design/filter_design.hpp"
#include "../../filter/iir_filter/coeff/ivantsov_coeff.hpp"
#include "../../filter/iir_filter/iir_base.hpp"
#include "../../filter/reverse_iir_filter/reverse_first_order_iir_base.hpp"
#include "../../filter/reverse_iir_filter/reverse_iir_base.hpp"

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
            freq_ = freq;
            to_update_ = true;
        }

        void setOrder(const size_t order) {
            order_ = order;
            to_update_ = true;
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
            first_order_filter_.prepare(max_num_samples, num_channels);
            filter_[0].setNumStage(kSecondOrderNumStage + extra_stage_);
            filter_[0].prepare(max_num_samples, num_channels);
            filter_[1].setNumStage(kSecondOrderNumStage + extra_stage_);
            filter_[1].prepare(max_num_samples, num_channels);
            forward_filter_[0].prepare(num_channels);
            forward_filter_[1].prepare(num_channels);

            const auto max_delay = static_cast<FloatType>((1 << (kSecondOrderNumStage + extra_stage_ + 2)) + 2);

            delay_.prepare(sample_rate, max_num_samples, num_channels, max_delay / static_cast<FloatType>(sample_rate));
            // update delay and filter order after prepare
            c_order_ = 0;
            to_update_ = true;
        }

        void reset() {
            delay_.reset();
            first_order_filter_.reset();
            for (auto& filter : filter_) {
                filter.reset();
            }
            for (auto& filter : forward_filter_) {
                filter.reset();
            }
        }

        void prepareBuffer() {
            if (std::exchange(to_update_, false)) {
                if (c_order_ != order_) {
                    c_order_ = order_;
                    updateOrder(c_order_);
                }
                updateFreq(freq_);
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
                default: {
                }
            }

            delay_.process(high_buffer, num_samples);
            for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                zldsp::vector::sub(high_buffer[chan], low_buffer[chan], num_samples);
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

        int getMaxLatency() const {
            return static_cast<int>(1 << (kSecondOrderNumStage + extra_stage_ + 2)) + 2;
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

        double freq_{1000.0};

        size_t order_{2};
        size_t c_order_{0};
        bool to_update_{true};

        static constexpr double order2q = 0.7071067811865476; // np.sqrt(2) / 2

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
            if (c_order_ != 1 && c_order_ != 2 && c_order_ != 4) {
                return;
            }

            std::array<std::array<double, 5>, 2> coeffs{};
            const auto num_filters = zldsp::filter::FilterDesign::updateCoeffs<zldsp::filter::IvantsovCoeff>(
                zldsp::filter::kLowPass, c_order_, freq, sample_rate_, 0.0, order2q, coeffs);
            for (size_t i = 0; i < num_filters; ++i) {
                forward_filter_[i].updateFromBiquad(coeffs[i]);
            }
            if (c_order_ == 1) {
                const auto& coeff = coeffs[0];
                first_order_filter_.updateFromBiquad({coeff[0], coeff[2], coeff[3]});
            } else {
                for (size_t i = 0; i < num_filters; ++i) {
                    filter_[i].updateFromBiquad(coeffs[i]);
                }
            }
        }
    };
}
