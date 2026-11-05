// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cassert>
#include <cmath>
#include <span>
#include <vector>

#include "../../vector/vector.hpp"

namespace zldsp::analyzer {
    class SpectrumDecayer {
    public:
        explicit SpectrumDecayer() = default;

        void prepare(const size_t fft_size) {
            prepareSpectrum(fft_size / 2 + 1);
        }

        void prepareSpectrum(const size_t spectrum_size) {
            state_.resize(spectrum_size);
            std::ranges::fill(state_.begin(), state_.end(), -240.f);
        }

        void setDecaySpeed(const float refresh_rate, const float min_db, const float decay_second) {
            // match sqr_mag_to_db's floor, below the -140 dB plot margin
            constexpr float floor_db = -240.0f;
            constexpr float start_db = 0.0f;
            if (!std::isfinite(refresh_rate) || !std::isfinite(decay_second) || !std::isfinite(min_db) ||
                refresh_rate <= 0.0f || decay_second <= 0.0f || min_db <= floor_db || min_db >= start_db) {
                p_ = 1.0f;
                return;
            }
            const float n_frames = refresh_rate * decay_second;

            const float start_dist = start_db - floor_db;
            const float target_dist = min_db - floor_db;

            const float ratio = target_dist / start_dist;

            p_ = -std::expm1(std::log(ratio) / n_frames);
            p_ = std::clamp(p_, 0.0f, 1.0f);
        }

        void decay(std::span<float> spectrum_db, const bool frozen = false) {
            assert(spectrum_db.size() == state_.size());
            namespace hn = hwy::HWY_NAMESPACE;

            static constexpr hn::ScalableTag<float> d;
            static constexpr size_t lanes = hn::MaxLanes(d);

            float* const HWY_RESTRICT spectrum_ptr = spectrum_db.data();
            float* const HWY_RESTRICT state_ptr = state_.data();
            size_t i = 0;

            if (frozen) {
                for (; i + lanes <= spectrum_db.size(); i += lanes) {
                    const auto spec = hn::LoadU(d, spectrum_ptr + i);
                    const auto state = hn::Load(d, state_ptr + i);
                    const auto v = hn::Max(spec, state);
                    hn::StoreU(v, d, spectrum_ptr + i);
                    hn::Store(v, d, state_ptr + i);
                }
                for (; i < spectrum_db.size(); ++i) {
                    const float v = std::max(spectrum_ptr[i], state_ptr[i]);
                    spectrum_ptr[i] = v;
                    state_ptr[i] = v;
                }
            } else {
                const auto v_p = hn::Set(d, p_);
                for (; i + lanes <= spectrum_db.size(); i += lanes) {
                    const auto spec = hn::LoadU(d, spectrum_ptr + i);
                    const auto state = hn::Load(d, state_ptr + i);
                    const auto diff = hn::Sub(spec, state);
                    const auto smoothed = hn::MulAdd(v_p, diff, state);
                    const auto v = hn::Max(spec, smoothed);
                    hn::StoreU(v, d, spectrum_ptr + i);
                    hn::Store(v, d, state_ptr + i);
                }
                for (; i < spectrum_db.size(); ++i) {
                    const float smoothed = state_ptr[i] + p_ * (spectrum_ptr[i] - state_ptr[i]);
                    const float v = std::max(spectrum_ptr[i], smoothed);
                    spectrum_ptr[i] = v;
                    state_ptr[i] = v;
                }
            }
        }

    private:
        vector::aligned_vector<float> state_{};
        float p_{1.0f};
    };
}
