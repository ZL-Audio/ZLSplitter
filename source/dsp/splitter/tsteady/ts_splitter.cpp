// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#include "ts_splitter.hpp"

namespace zlSplitter {
    template<typename FloatType>
    void TSSplitter<FloatType>::prepare(const juce::dsp::ProcessSpec &spec) {
        if (spec.sampleRate <= 50000) {
            timeMedian.resize(2048);
        } else if (spec.sampleRate <= 100000) {
            timeMedian.resize(4096);
        } else {
            timeMedian.resize(8192);
        }
    }

    template
    class TSSplitter<float>;

    template
    class TSSplitter<double>;
} // zlSplitter