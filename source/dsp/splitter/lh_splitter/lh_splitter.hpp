// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <array>
#include <span>
#include <utility>

#include "../../chore/smoothed_value.hpp"
#include "../../filter/filter_design/filter_design.hpp"
#include "../../filter/iir_filter/coeff/ivantsov_coeff.hpp"
#include "../../filter/iir_filter/iir_base.hpp"

namespace zldsp::splitter {
    template<typename FloatType>
    class LHSplitter {
    public:
        LHSplitter() = default;

        void setMix(const FloatType mix) {
            mix_.setTarget(mix);
        }

        void setFreq(const double freq) {
            freq_ = freq;
            to_update_freq_ = true;
        }

        void setOrder(const size_t order) {
            order_ = order;
            to_update_order_ = true;
        }

        void prepare(const double sample_rate, const size_t num_channels) {
            sample_rate_ = sample_rate;
            mix_.prepare(sample_rate, 0.1);
            c_freq_.prepare(sample_rate, 0.125);
            c_freq_.setCurrentAndTarget(freq_);
            c_order_ = order_;

            for (auto &filter: low_filters_) {
                filter.prepare(num_channels);
            }
            for (auto &filter: allpass_filters_) {
                filter.prepare(num_channels);
            }
            updateFreq(c_freq_.getCurrent());
        }

        void reset() {
            for (auto& filter : low_filters_) {
                filter.reset();
            }
            for (auto& filter : allpass_filters_) {
                filter.reset();
            }
        }

        void prepareBuffer() {
            if (std::exchange(to_update_freq_, false)) {
                c_freq_.setTarget(freq_);
            }
            if (std::exchange(to_update_order_, false)) {
                if (c_order_ != order_) {
                    c_order_ = order_;
                    reset();
                    updateFreq(c_freq_.getCurrent());
                }
            }
        }

        void process(std::span<FloatType *> in_buffer,
                     std::span<FloatType *> low_buffer,
                     std::span<FloatType *> high_buffer,
                     const size_t num_samples) {
            if (num_samples == 0) {
                return;
            }
            if (c_order_ == 4) {
                processFilters<2>(in_buffer, low_buffer, high_buffer, num_samples);
            } else {
                processFilters<1>(in_buffer, low_buffer, high_buffer, num_samples);
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
        std::array<zldsp::filter::IIRBase<double>, 4> low_filters_;
        std::array<zldsp::filter::IIRBase<double>, 2> allpass_filters_;
        double sample_rate_{48000.0};

        zldsp::chore::SmoothedValue<FloatType, chore::SmoothedTypes::kLin> mix_{static_cast<FloatType>(0)};

        double freq_{1000.0};
        zldsp::chore::SmoothedValue<double, zldsp::chore::kFixMul> c_freq_{1000.0};
        bool to_update_freq_{false};

        size_t order_{2};
        size_t c_order_{2};
        bool to_update_order_{true};

        static constexpr double kButterworthQ = 0.7071067811865476;

        void updateFreq(const double freq) {
            std::array<std::array<double, 5>, 2> coeffs{};
            const auto num_filters = zldsp::filter::FilterDesign::updateCoeffs<zldsp::filter::IvantsovCoeff>(
                zldsp::filter::kLowPass, c_order_, freq, sample_rate_, 0.0, kButterworthQ, coeffs);
            for (size_t i = 0; i < num_filters; ++i) {
                const auto &c = coeffs[i];
                low_filters_[2 * i].updateFromBiquad(c);
                if (c_order_ == 1) {
                    low_filters_[2 * i + 1].updateFromBiquad({c[0], 0.0, c[3], c[2], 0.0});
                    allpass_filters_[i].updateFromBiquad({c[0], 0.0, c[0], 1.0, 0.0});
                } else {
                    low_filters_[2 * i + 1].updateFromBiquad({c[0], c[1], c[4], c[3], c[2]});
                    allpass_filters_[i].updateFromBiquad({c[0], c[1], c[1], c[0], 1.0});
                }
            }
        }

        template<size_t NumFilters>
        void processFilters(std::span<FloatType *> in_buffer,
                            std::span<FloatType *> low_buffer,
                            std::span<FloatType *> high_buffer,
                            const size_t num_samples) {
            if (c_freq_.isSmoothing()) {
                for (size_t i = 0; i < num_samples; ++i) {
                    updateFreq(c_freq_.getNext());
                    for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                        auto low = static_cast<double>(in_buffer[chan][i]);
                        auto reference = low;
                        for (size_t f = 0; f < NumFilters; ++f) {
                            low = low_filters_[2 * f].processSample(chan, low);
                            low = low_filters_[2 * f + 1].processSample(chan, low);
                            reference = allpass_filters_[f].processSample(chan, reference);
                        }
                        low_buffer[chan][i] = static_cast<FloatType>(low);
                        high_buffer[chan][i] = static_cast<FloatType>(reference - low);
                    }
                }
            } else {
                for (size_t chan = 0; chan < in_buffer.size(); ++chan) {
                    std::array<zldsp::filter::IIRBase<double>::LocalState, 2 * NumFilters> low{};
                    std::array<zldsp::filter::IIRBase<double>::LocalState, NumFilters> allpass{};
                    for (size_t f = 0; f < NumFilters; ++f) {
                        low[2 * f] = low_filters_[2 * f].getLocalState(chan);
                        low[2 * f + 1] = low_filters_[2 * f + 1].getLocalState(chan);
                        allpass[f] = allpass_filters_[f].getLocalState(chan);
                    }
                    const auto in_chan = in_buffer[chan];
                    const auto low_chan = low_buffer[chan];
                    const auto high_chan = high_buffer[chan];
                    for (size_t i = 0; i < num_samples; ++i) {
                        auto low_x = static_cast<double>(in_chan[i]);
                        auto reference = low_x;
                        for (size_t f = 0; f < NumFilters; ++f) {
                            low_x = low[2 * f].processSample(low_x);
                            low_x = low[2 * f + 1].processSample(low_x);
                            reference = allpass[f].processSample(reference);
                        }
                        low_chan[i] = static_cast<FloatType>(low_x);
                        high_chan[i] = static_cast<FloatType>(reference - low_x);
                    }
                    for (size_t f = 0; f < NumFilters; ++f) {
                        low_filters_[2 * f].saveLocalState(chan, low[2 * f]);
                        low_filters_[2 * f + 1].saveLocalState(chan, low[2 * f + 1]);
                        allpass_filters_[f].saveLocalState(chan, allpass[f]);
                    }
                }
            }
        }
    };
}
