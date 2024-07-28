// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

// Based on:
// http://stackoverflow.com/a/5970314/383299
// https://gist.github.com/ashelly/5665911
// https://github.com/craffel/median-filter
// https://github.com/suomela/median-filter

#ifndef MEDIAN_HEAP_FILTER_HPP
#define MEDIAN_HEAP_FILTER_HPP

namespace zlMedianFilter {
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
            return static_cast<size_t>(dataSize);
        }

        /**
         * clear internal state
         */
        void clear() {
            std::fill(data.begin(), data.end(), static_cast<T>(0));
            std::fill(pos.begin(), pos.end(), 0);
            std::fill(heap.begin(), heap.end(), 0);
            idx = 0;
            minCt = 0;
            maxCt = 0;
            dataSize = 0;
            auto nItems = static_cast<int>(N);
            while (nItems--) {
                pos[static_cast<size_t>(nItems)] = ((nItems + 1) / 2) * ((nItems & 1) ? -1 : 1);
                heap[static_cast<size_t>(centerPos + pos[static_cast<size_t>(nItems)])] = nItems;
            }
        }

        /**
         * insert a new data into the circular buffer
         * @param v new data
         */
        void insert(T v) {
            dataSize = (dataSize + 1) % static_cast<int>(N);
            const int p = pos[static_cast<size_t>(idx)];
            T old = data[static_cast<size_t>(idx)];
            data[static_cast<size_t>(idx)] = v;
            idx = (idx + 1) % static_cast<int>(N);
            // New item is in minheap
            if (p > 0) {
                if (minCt < static_cast<int>(N - 1) / 2) {
                    minCt++;
                } else if (v > old) {
                    minSortDown(p);
                    return;
                }
                if (minSortUp(p) && mmCmpExch(0, -1)) {
                    maxSortDown(-1);
                }
            }
            // New item is in maxheap
            else if (p < 0) {
                if (maxCt < static_cast<int>(N) / 2) {
                    maxCt++;
                } else if (v < old) {
                    maxSortDown(p);
                    return;
                }
                if (maxSortUp(p) && minCt && mmCmpExch(1, 0)) {
                    minSortDown(1);
                }
            }
            // New item is at median
            else {
                if (maxCt && maxSortUp(-1)) {
                    maxSortDown(-1);
                }
                if (minCt && minSortUp(1)) {
                    minSortDown(1);
                }
            }
        }

        /**
         *
         * @return current median
         */
        T getMedian() {
            if (minCt < maxCt) {
                return (data[static_cast<size_t>(heap[static_cast<size_t>(centerPos)])] + data[static_cast<size_t>(heap[static_cast<size_t>(centerPos - 1)])]) / 2;
            } else {
                return data[static_cast<size_t>(heap[static_cast<size_t>(centerPos)])];
            }
        }

    private:
        std::array<T, N> data;
        std::array<int, N> pos;
        std::array<int, N> heap;
        static constexpr int centerPos = static_cast<int>(N / 2);
        // Position in circular queue
        int idx{0};
        // Count of items in min heap
        int minCt{0};
        // Count of items in max heap
        int maxCt{0};
        // Count of items
        int dataSize{0};

        // Swaps items i&j in heap, maintains indexes
        int mmexchange(const int i, const int j) {
            const auto ii = static_cast<size_t>(centerPos + i);
            const auto jj = static_cast<size_t>(centerPos + j);
            const auto t = heap[ii];
            heap[ii] = heap[jj];
            heap[jj] = t;
            pos[static_cast<size_t>(heap[ii])] = i;
            pos[static_cast<size_t>(heap[jj])] = j;
            return 1;
        }

        // Maintains minheap property for all items below i.
        void minSortDown(int i) {
            for (i *= 2; i <= minCt; i *= 2) {
                if (i < minCt && mmless(i + 1, i)) {
                    ++i;
                }
                if (!mmCmpExch(i, i / 2)) {
                    break;
                }
            }
        }

        // Maintains maxheap property for all items below i. (negative indexes)
        void maxSortDown(int i) {
            for (i *= 2; i >= -maxCt; i *= 2) {
                if (i > -maxCt && mmless(i, i - 1)) {
                    --i;
                }
                if (!mmCmpExch(i / 2, i)) {
                    break;
                }
            }
        }

        // Returns 1 if heap[i] < heap[j]
        inline int mmless(const int i, const int j) {
            return (data[static_cast<size_t>(heap[static_cast<size_t>(centerPos + i)])] < data[static_cast<size_t>(heap[static_cast<size_t>(centerPos + j)])]);
        }

        // Swaps items i&j if i<j; returns true if swapped
        inline int mmCmpExch(const int i, const int j) {
            return (mmless(i, j) && mmexchange(i, j));
        }

        // Maintains minheap property for all items above i, including median
        // Returns true if median changed
        inline int minSortUp(int i) {
            while (i > 0 && mmCmpExch(i, i / 2)) {
                i /= 2;
            }
            return (i == 0);
        }

        // Maintains maxheap property for all items above i, including median
        // Returns true if median changed
        inline int maxSortUp(int i) {
            while (i < 0 && mmCmpExch(i / 2, i)) {
                i /= 2;
            }
            return (i == 0);
        }
    };
} // zlMedianFilter

#endif //MEDIAN_HEAP_FILTER_HPP
