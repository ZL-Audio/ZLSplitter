// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <atomic>

#include "ideal_base.hpp"
#include "coeff/ideal_coeff.hpp"
#include "../filter_design/filter_design.hpp"
#include "../../chore/decibels.hpp"
#include "../../vector/kfr_import.hpp"

namespace zldsp::filter {
    /**
     * an ideal prototype filter which holds coeffs for calculating responses
     * @tparam FloatType the float type of input audio buffer
     * @tparam FilterSize the number of cascading filters
     */
    template<typename FloatType, size_t FilterSize>
    class Ideal {
    public:
        explicit Ideal() = default;

        void prepare(const double sample_rate) {
            fs_.store(sample_rate, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        bool prepareBuffer() {
            if (to_update_.exchange(false, std::memory_order::acquire)) {
                current_filter_num_ = updateIIRCoeffs(
                    filter_type_.load(std::memory_order::relaxed),
                    order_.load(std::memory_order::relaxed),
                    freq_.load(std::memory_order::relaxed), fs_.load(std::memory_order::relaxed),
                    gain_.load(std::memory_order::relaxed), q_.load(std::memory_order::relaxed),
                    coeffs_);
                return true;
            }
            return false;
        }

        void setFreq(const FloatType x) {
            freq_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        FloatType getFreq() const {
            return static_cast<FloatType>(freq_.load(std::memory_order::relaxed));
        }

        void setGain(const FloatType x) {
            if (std::abs(static_cast<double>(x) - gain_.load()) > 1e-6) {
                gain_.store(x, std::memory_order::relaxed);
                to_update_.store(true, std::memory_order::release);
            }
        }

        FloatType getGain() const {
            return static_cast<FloatType>(gain_.load(std::memory_order::relaxed));
        }

        void setQ(const FloatType x) {
            if (std::abs(static_cast<double>(x) - q_.load()) > 1e-6) {
                q_.store(x, std::memory_order::relaxed);
                to_update_.store(true, std::memory_order::release);
            }
        }

        FloatType getQ() const {
            return static_cast<FloatType>(q_.load(std::memory_order::relaxed));
        }

        void setFilterType(const FilterType x) {
            filter_type_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        FilterType getFilterType() const {
            return filter_type_.load(std::memory_order::relaxed);
        }

        void setOrder(const size_t x) {
            order_.store(x, std::memory_order::relaxed);
            to_update_.store(true, std::memory_order::release);
        }

        size_t getOrder() const {
            return order_.load(std::memory_order::relaxed);
        }

        void updateResponse(std::span<std::complex<FloatType>> wis,
                            std::span<std::complex<FloatType>> response) {
            if (current_filter_num_ == 0) {
                std::fill(response.begin(), response.end(), std::complex(FloatType(1), FloatType(0)));
            } else {
                IdealBase<FloatType>::updateResponse(coeffs_[0], wis, response);
                for (size_t i = 1; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::multiplyResponse(coeffs_[i], wis, response);
                }
            }
        }

        void multiplyResponse(std::span<std::complex<FloatType>> wis,
                              std::span<std::complex<FloatType>> response) {
            if (current_filter_num_ > 0) {
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::multiplyResponse(coeffs_[i], wis, response);
                }
            }
        }

        void updateMagnitude(const std::span<const FloatType> ws,
                             std::span<FloatType> dbs) {
            if (current_filter_num_ == 0) {
                std::fill(dbs.begin(), dbs.end(), FloatType(0));
            } else {
                IdealBase<FloatType>::updateMagnitude(coeffs_[0], ws, dbs);
                for (size_t i = 1; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::multiplyMagnitude(coeffs_[i], ws, dbs);
                }
            }
        }

        void multiplyMagnitude(const std::span<const FloatType> ws,
                               std::span<FloatType> dbs) {
            if (current_filter_num_ > 0) {
                for (size_t i = 0; i < current_filter_num_; ++i) {
                    IdealBase<FloatType>::multiplyMagnitude(coeffs_[i], ws, dbs);
                }
            }
        }

        FloatType getDB(FloatType w) {
            double g0 = 1.0;
            for (size_t i = 0; i < current_filter_num_; ++i) {
                g0 *= IdealBase<FloatType>::getMagnitude(coeffs_[i], w);
            }
            return static_cast<FloatType>(chore::gainToDecibels(g0));
        }

    private:
        std::array<std::array < double, 6>
        ,
        FilterSize
        >
        coeffs_ {
        };
        std::atomic<size_t> order_{2};
        size_t current_filter_num_{1};
        std::atomic<double> freq_{1000.0}, gain_{0.0}, q_{0.707};
        std::atomic<double> fs_{48000.0};
        std::atomic<FilterType> filter_type_ = FilterType::kPeak;
        std::atomic<bool> to_update_{false};

        static size_t updateIIRCoeffs(const FilterType filterType, const size_t n,
                                      const double f, const double fs, const double g0, const double q0,
                                      std::array<std::array < double, 6>, FilterSize> &coeffs) {
            return FilterDesign::updateCoeffs < FilterSize,
                   IdealCoeff::get1LowShelf, IdealCoeff::get1HighShelf, IdealCoeff::get1TiltShelf,
                   IdealCoeff::get1LowPass, IdealCoeff::get1HighPass,
                   IdealCoeff::get2Peak,
                   IdealCoeff::get2LowShelf, IdealCoeff::get2HighShelf, IdealCoeff::get2TiltShelf,
                   IdealCoeff::get2LowPass, IdealCoeff::get2HighPass,
                   IdealCoeff::get2BandPass, IdealCoeff::get2Notch > (
                       filterType, n, f, fs, g0, q0, coeffs);
        }
    };
}