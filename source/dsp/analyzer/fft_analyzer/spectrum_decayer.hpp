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
#include <vector>
#include <algorithm>

namespace zldsp::analyzer {
    class SpectrumDecayer {
    public:
        explicit SpectrumDecayer() = default;

        void prepare(const size_t fft_size) {
            state_.resize(fft_size / 2 + 1);
            std::ranges::fill(state_.begin(), state_.end(), -240.f);
        }

        void setDecaySpeed(const float refresh_rate, const float decay_per_second) {
            decay_per_call_ = decay_per_second / refresh_rate;
        }

        void decay(std::span<float> spectrum_db, const bool frozen = false) {
            if (frozen) {
                for (size_t i = 0; i < spectrum_db.size(); ++i) {
                    spectrum_db[i] = std::max(spectrum_db[i], state_[i]);
                    state_[i] = spectrum_db[i];
                }
            } else {
                for (size_t i = 0; i < spectrum_db.size(); ++i) {
                    const float v = std::max(spectrum_db[i],
                                             std::max(state_[i] + decay_per_call_,
                                                      (spectrum_db[i] + state_[i]) * .5f));
                    spectrum_db[i] = v;
                    state_[i] = v;
                }
            }
        }

    private:
        std::vector<float> state_{};
        float decay_per_call_{0.};
    };
}
