// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>
#include <algorithm>

namespace zldsp::chore {
    enum SmoothedTypes {
        kLin, kMul, kFixLin, kFixMul
    };

    template<typename FloatType, SmoothedTypes SmoothedType = SmoothedTypes::kLin>
    class SmoothedValue {
    public:
        SmoothedValue() {
            setCurrentAndTarget(FloatType(0));
        }

        explicit SmoothedValue(const FloatType x) {
            setCurrentAndTarget(x);
        }

        void prepare(const double sample_rate, const double ramp_length_in_seconds) {
            switch (SmoothedType) {
                case kLin:
                case kMul: {
                    max_count_ = static_cast<int>(sample_rate * ramp_length_in_seconds);
                    break;
                }
                case kFixLin: {
                    inc_ = static_cast<FloatType>(1.0 / (sample_rate * ramp_length_in_seconds));
                    increase_inc_ = inc_;
                    decrease_inc_ = -inc_;
                    break;
                }
                case kFixMul: {
                    inc_ = static_cast<FloatType>(std::pow(2.0, 1.0 / (sample_rate * ramp_length_in_seconds)));
                    increase_inc_ = inc_;
                    decrease_inc_ = FloatType(1.0) / inc_;
                    break;
                }
            }
            setCurrentAndTarget(getTarget());
        }

        void setTarget(const FloatType x) {
            target_ = x;
            if (std::abs(current_ - target_) < FloatType(1e-10)) {
                count_ = 0;
                return;
            }
            switch (SmoothedType) {
                case kLin: {
                    inc_ = (target_ - current_) / static_cast<FloatType>(max_count_);
                    count_ = max_count_;
                    break;
                }
                case kMul: {
                    inc_ = std::exp(std::log(target_ / current_) / static_cast<FloatType>(max_count_));
                    count_ = max_count_;
                    break;
                }
                case kFixLin:
                case kFixMul: {
                    count_ = 1;
                    is_increasing_ = target_ > current_;
                    break;
                }
            }
        }

        void setCurrentAndTarget(const FloatType x) {
            current_ = x;
            target_ = x;
            count_ = 0;
        }

        FloatType getCurrent() const { return current_; }

        FloatType getTarget() const { return target_; }

        [[nodiscard]] bool isSmoothing() const { return count_ > 0; }

        FloatType getNext() {
            if (count_ == 0) { return current_; }
            switch (SmoothedType) {
                case kLin: {
                    current_ += inc_;
                    count_ -= 1;
                    return current_;
                }
                case kMul: {
                    current_ *= inc_;
                    count_ -= 1;
                    return current_;
                }
                case kFixLin: {
                    if (is_increasing_) {
                        current_ += increase_inc_;
                        if (current_ > target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    } else {
                        current_ += decrease_inc_;
                        if (current_ < target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    }
                    return current_;
                }
                case kFixMul: {
                    if (is_increasing_) {
                        current_ *= increase_inc_;
                        if (current_ > target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    } else {
                        current_ *= decrease_inc_;
                        if (current_ < target_) {
                            current_ = target_;
                            count_ = 0;
                        }
                    }
                    return current_;
                }
                default: return current_;
            }
        }

    private:
        FloatType current_{}, target_{}, inc_{};
        FloatType increase_inc_{}, decrease_inc_{};
        int max_count_{}, count_{};
        bool is_increasing_{};
    };
}
