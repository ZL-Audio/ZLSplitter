// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef FIFODELAY_HPP
#define FIFODELAY_HPP

#include <juce_dsp/juce_dsp.h>

namespace zlDelay {
    template<typename SampleType>
    class FIFODelay {
    public:
        FIFODelay() {
            setMaximumDelayInSamples(1);
        }

        explicit FIFODelay(const int maxDelay) {
            setMaximumDelayInSamples(maxDelay);
        }

        void setMaximumDelayInSamples(const int x) {
            maximumDelay = x;
            states.resize(static_cast<size_t>(x));
            reset();
        }

        void setDelay(const int x) {
            numDelay = x;
        }

        void reset() {
            std::fill(states.begin(), states.end(), SampleType(0));
            pos = 0;
        }

        SampleType push(SampleType x) {
            const auto current = states[static_cast<size_t>(pos)];
            states[static_cast<size_t>(pos)] = x;
            pos = (pos + 1) % numDelay;
            return current;
        }

    private:
        int maximumDelay{1}, numDelay{1};
        std::vector<SampleType> states;
        int pos{0};
    };
}

#endif //FIFODELAY_HPP
