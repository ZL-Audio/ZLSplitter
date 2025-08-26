// Copyright (C) 2025 - zsliu98
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

namespace zlpanel::multilingual::de {
    static constexpr std::array kTexts = {
        "Drücken: Eingang -> Output 1, Stille -> Output 2.",
        "Drücken: Left -> Output 1, Right -> Output 2.",
        "Drücken: Mid -> Output 1, Side -> Output 2.",
        "Drücken: Tiefen -> Output 1, Höhen -> Output 2.",
        "Drücken: Transienten -> Output 1, Stationär -> Output 2.",
        "Drücken: Spitzen -> Output 1, Stationär -> Output 2.",
        "Passen Sie das Mischverhältnis von Output 1 und Output 2 an.",
        "Wählen Sie den Filtertyp für die Tiefen-/Höhen-Trennung.",
        "Wählen Sie die Flankensteilheit des Filters für die Tiefen-/Höhen-Trennung.",
        "Passen Sie die Grenzfrequenz des Filters für die Tiefen-/Höhen-Trennung an.",
        "Passen Sie die Balance der Transient-/Stationär-Trennung an. Je kleiner die Balance, desto weniger Transientsignal und mehr stationäres Signal, und umgekehrt.",
        "Passen Sie die Stärke der Transient-/Stationär-Trennung an. Je kleiner die Stärke, desto sanfter die Trennung.",
        "Passen Sie die Haltezeit der Transient-/Stationär-Trennung an. Je größer die Haltezeit, desto langsamer der Abfall des Transientsignals.",
        "Passen Sie die spektrale Glätte der Transient-/Stationär-Trennung an.",
        "Passen Sie die Balance der Spitzen-/Stationär-Trennung an. Je kleiner die Balance, desto weniger Spitzensignal und mehr stationäres Signal, und umgekehrt.",
        "Passen Sie den Attack der Spitzen-/Stationär-Trennung an.",
        "Passen Sie die Haltezeit der Spitzen-/Stationär-Trennung an. Je größer die Haltezeit, desto langsamer der Abfall des Spitzensignals.",
        "Passen Sie die Glätte der Spitzen-/Stationär-Trennung an. Je größer die Glätte, desto größer die RMS-Fenstergröße.",
        "Drücken: Spektrumanalysator einschalten.",
        "Drücken: Amplitudenanalysator einschalten.",
        "Drücken: tausche Output 1 und Output 2.",
        "Loslassen: Plugin umgehen.",
        "Doppelklick: Öffnet die Benutzeroberflächen-Einstellungen."
    };
}
