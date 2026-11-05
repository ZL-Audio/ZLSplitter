// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "fir_base.hpp"
#include "../ideal_filter/ideal_filter.hpp"
#include "fir_helper.hpp"

namespace zldsp::filter {
    template<typename FloatType, size_t FilterNum, size_t FilterSize, size_t DefaultFFTOrder = 10>
    class FIRFilter : public FIRBase<FloatType, DefaultFFTOrder> {
    public:
        explicit FIRFilter(std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal_fs,
                           std::vector<size_t> &on_indices)
            : ideal_fs_(ideal_fs), on_indices_(on_indices) {
        }

        void update() {
            if (on_indices_.empty()) {
                std::fill(corrections_.begin(), corrections_.end(), 1.f);
                return;
            }
            ideal_fs_[on_indices_[0]].updateResponse(wis_, response_);
            for (size_t i = 1; i < on_indices_.size(); ++i) {
                ideal_fs_[on_indices_[i]].multiplyResponse(wis_, response_);
            }
            for (size_t i = 0; i < response_.size(); ++i) {
                const auto magnitude = std::abs(response_[i]);
                corrections_[i] = static_cast<float>(magnitude);
            }
        }

    protected:
        std::array<Ideal<FloatType, FilterSize>, FilterNum> &ideal_fs_;
        std::vector<size_t> &on_indices_;

        std::vector<std::complex<FloatType>> wis_, response_;
        zldsp::vector::aligned_vector<float> corrections_{};

        void setOrder(const size_t num_channels, const size_t order) override {
            FIRBase<FloatType, DefaultFFTOrder>::setFFTOrder(num_channels, order);

            wis_.resize(FIRBase<FloatType, DefaultFFTOrder>::num_bins_);
            response_.resize(FIRBase<FloatType, DefaultFFTOrder>::num_bins_);
            corrections_.resize(FIRBase<FloatType, DefaultFFTOrder>::num_bins_);
            FIRBase<FloatType, DefaultFFTOrder>::reset();

            calculateWsForPrototype<FloatType>(wis_);
            update();
        }

        void processSpectrum() override {
            for (auto& plane : this->fft_data_) {
                zldsp::vector::multiply(plane.data(), corrections_.data(), corrections_.size());
            }
        }
    };
}