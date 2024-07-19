// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplit
//
// ZLSplit is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplit. If not, see <https://www.gnu.org/licenses/>.

#ifndef REVERSE_IIR_BASE_HPP
#define REVERSE_IIR_BASE_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlReverseIIR {
    template<typename SampleType>
    class ReverseIIRBase {

        explicit ReverseIIRBase(const size_t stage) : numStage(stage) {
            delays.resize(stage);
            u.resize(stage);
            v.resize(stage);
            as.resize(stage);
            bs.resize(stage);
        }

        void updateFromBiquad(const std::array<double, 3> &a, const std::array<double, 3> &b) {
            as[0] = -static_cast<SampleType>(a[1] / 2);
            bs[0] = -static_cast<SampleType>(std::sqrt(a[2] - a[1] * a[1] / 4));
            
        }

    private:
        size_t numStage = 10;
        std::vector<juce::dsp::DelayLine<SampleType>> delays;
        std::vector<SampleType> u, v, as, bs;
    };
}

#endif //REVERSE_IIR_BASE_HPP
