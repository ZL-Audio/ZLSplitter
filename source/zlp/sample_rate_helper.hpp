// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstddef>

namespace zlp {
    inline size_t getScaledOrder(const double sample_rate, const size_t order_at_50_khz) {
        if (sample_rate <= 12500.0) {
            return order_at_50_khz - 2;
        }
        if (sample_rate <= 25000.0) {
            return order_at_50_khz - 1;
        }
        if (sample_rate <= 50000.0) {
            return order_at_50_khz;
        }
        if (sample_rate <= 100000.0) {
            return order_at_50_khz + 1;
        }
        if (sample_rate <= 200000.0) {
            return order_at_50_khz + 2;
        }
        return order_at_50_khz + 3;
    }
}
