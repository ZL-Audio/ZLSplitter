// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef FIFODELAY_HPP
#define FIFODELAY_HPP

namespace zlReverseIIR {
    template<typename SampleType>
    class FIFODelay {
    public:
        FIFODelay(const size_t maxDelay = 1) {
            setMaximumDelayInSamples(maxDelay);
        }

        void setMaximumDelayInSamples(const size_t x) {
            maximumDelay = x;
            states.resize(x);
            reset();
        }

        void setDelay(const size_t x) {
            numDelay = x;
        }

        void reset() {
            std::fill(states.begin(), states.end(), SampleType(0));
            pos = 0;
        }

        SampleType push(SampleType x) {
            const auto current = states[pos];
            states[pos] = x;
            pos = (pos + 1) % numDelay;
            return current;
        }

    private:
        size_t maximumDelay, numDelay;
        std::vector<SampleType> states;
        size_t pos = 0;
    };
}

#endif //FIFODELAY_HPP
