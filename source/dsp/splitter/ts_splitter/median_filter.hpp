// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

// Based on:
// http://stackoverflow.com/a/5970314/383299
// https://gist.github.com/ashelly/5665911
// https://github.com/craffel/median-filter
// https://github.com/suomela/median-filter

#pragma once

#include <array>

namespace zldsp::splitter {
    /**
     * a median filter based on min/max heap
     * @tparam T type of data
     * @tparam N size of the filter
     */
    template<typename T, size_t N>
    class HeapFilter {
    public:
        HeapFilter() {
            clear();
        }

        /**
         *
         * @return size of data
         */
        [[nodiscard]] size_t size() const {
            return static_cast<size_t>(data_size_);
        }

        /**
         * clear internal state
         */
        void clear() {
            std::fill(data_.begin(), data_.end(), static_cast<T>(0));
            std::fill(pos_.begin(), pos_.end(), 0);
            std::fill(heap_.begin(), heap_.end(), 0);
            idx_ = 0;
            min_ct_ = 0;
            max_ct_ = 0;
            data_size_ = 0;
            auto nItems = static_cast<int>(N);
            while (nItems--) {
                pos_[static_cast<size_t>(nItems)] = ((nItems + 1) / 2) * ((nItems & 1) ? -1 : 1);
                heap_[static_cast<size_t>(center_pos_ + pos_[static_cast<size_t>(nItems)])] = nItems;
            }
        }

        /**
         * insert a new data into the circular buffer
         * @param v new data
         */
        void insert(T v) {
            data_size_ = (data_size_ + 1) % static_cast<int>(N);
            const int p = pos_[static_cast<size_t>(idx_)];
            T old = data_[static_cast<size_t>(idx_)];
            data_[static_cast<size_t>(idx_)] = v;
            idx_ = (idx_ + 1) % static_cast<int>(N);
            // New item is in minheap
            if (p > 0) {
                if (min_ct_ < static_cast<int>(N - 1) / 2) {
                    min_ct_++;
                } else if (v > old) {
                    minSortDown(p);
                    return;
                }
                if (minSortUp(p) && mmCmpExchange(0, -1)) {
                    maxSortDown(-1);
                }
            }
            // New item is in maxheap
            else if (p < 0) {
                if (max_ct_ < static_cast<int>(N) / 2) {
                    max_ct_++;
                } else if (v < old) {
                    maxSortDown(p);
                    return;
                }
                if (maxSortUp(p) && min_ct_ && mmCmpExchange(1, 0)) {
                    minSortDown(1);
                }
            }
            // New item is at median
            else {
                if (max_ct_ && maxSortUp(-1)) {
                    maxSortDown(-1);
                }
                if (min_ct_ && minSortUp(1)) {
                    minSortDown(1);
                }
            }
        }

        /**
         *
         * @return current median
         */
        T getMedian() {
            if (min_ct_ < max_ct_) {
                return (data_[static_cast<size_t>(heap_[static_cast<size_t>(center_pos_)])] + data_[static_cast<size_t>(heap_[static_cast<size_t>(center_pos_ - 1)])]) / 2;
            } else {
                return data_[static_cast<size_t>(heap_[static_cast<size_t>(center_pos_)])];
            }
        }

    private:
        std::array<T, N> data_;
        std::array<int, N> pos_;
        std::array<int, N> heap_;
        static constexpr int center_pos_ = static_cast<int>(N / 2);
        // Position in circular queue
        int idx_{0};
        // Count of items in min heap
        int min_ct_{0};
        // Count of items in max heap
        int max_ct_{0};
        // Count of items
        int data_size_{0};

        // Swaps items i&j in heap, maintains indexes
        int mmExchange(const int i, const int j) {
            const auto ii = static_cast<size_t>(center_pos_ + i);
            const auto jj = static_cast<size_t>(center_pos_ + j);
            const auto t = heap_[ii];
            heap_[ii] = heap_[jj];
            heap_[jj] = t;
            pos_[static_cast<size_t>(heap_[ii])] = i;
            pos_[static_cast<size_t>(heap_[jj])] = j;
            return 1;
        }

        // Maintains minheap property for all items below i.
        void minSortDown(int i) {
            for (i *= 2; i <= min_ct_; i *= 2) {
                if (i < min_ct_ && mmLess(i + 1, i)) {
                    ++i;
                }
                if (!mmCmpExchange(i, i / 2)) {
                    break;
                }
            }
        }

        // Maintains maxheap property for all items below i. (negative indexes)
        void maxSortDown(int i) {
            for (i *= 2; i >= -max_ct_; i *= 2) {
                if (i > -max_ct_ && mmLess(i, i - 1)) {
                    --i;
                }
                if (!mmCmpExchange(i / 2, i)) {
                    break;
                }
            }
        }

        // Returns 1 if heap[i] < heap[j]
        inline int mmLess(const int i, const int j) {
            return (data_[static_cast<size_t>(heap_[static_cast<size_t>(center_pos_ + i)])] < data_[static_cast<size_t>(heap_[static_cast<size_t>(center_pos_ + j)])]);
        }

        // Swaps items i&j if i<j; returns true if swapped
        inline int mmCmpExchange(const int i, const int j) {
            return (mmLess(i, j) && mmExchange(i, j));
        }

        // Maintains minheap property for all items above i, including median
        // Returns true if median changed
        inline int minSortUp(int i) {
            while (i > 0 && mmCmpExchange(i, i / 2)) {
                i /= 2;
            }
            return (i == 0);
        }

        // Maintains maxheap property for all items above i, including median
        // Returns true if median changed
        inline int maxSortUp(int i) {
            while (i < 0 && mmCmpExchange(i / 2, i)) {
                i /= 2;
            }
            return (i == 0);
        }
    };
}
