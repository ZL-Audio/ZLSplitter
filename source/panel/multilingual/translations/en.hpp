// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

// This file is also dual licensed under the Apache License, Version 2.0. You may obtain a copy of the License at <http://www.apache.org/licenses/LICENSE-2.0>

#pragma once

#include <array>

namespace zlpanel::multilingual::en {
    static constexpr std::array kTexts = {
        "Press: Input -> Output 1, Silence -> Output 2.",
        "Press: Left -> Output 1, Right -> Output 2.",
        "Press: Mid -> Output 1, Side -> Output 2.",
        "Press: Low -> Output 1, High -> Output 2.",
        "Press: Transient -> Output 1, Steady -> Output 2.",
        "Press: Peak -> Output 1, Steady -> Output 2.",
        "Adjust the mix portion of Output 1 and Output 2.",
        "Choose low/high splitting filter type.",
        "Choose low/high splitting filter slope.",
        "Adjust low/high splitting filter cutoff frequency.",
        "Adjust the balance of transient/steady split. The smaller the balance, the less transient signal and the more steady signal, and vice versa.",
        "Adjust the strength of transient/steady split. The smaller the strength, the softer the split.",
        "Adjust the hold of transient/steady split. The larger the hold, the slower the decay of transient signal.",
        "Adjust the spectrum smoothness of transient/steady split.",
        "Adjust the balance of peak/steady split. The smaller the balance, the less peak signal and the more steady signal, and vice versa.",
        "Adjust the attack of peak/steady split.",
        "Adjust the hold of peak/steady split. The larger the hold, the slower the decay of peak signal.",
        "Adjust the smoothness of peak/steady split. The larger the smooth, the larger the RMS window size.",
        "Press: turn on spectrum analyzer.",
        "Press: turn on magnitude analyzer.",
        "Press: swap Output 1 and Output 2",
        "Release: bypass the plugin.",
        "Double click: open UI settings."
    };
}