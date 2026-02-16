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

namespace zlpanel::multilingual::it {
    static constexpr std::array kTexts = {
        "Pressione: Ingresso -> Output 1, Silenzio -> Output 2.",
        "Pressione: Left -> Output 1, Right -> Output 2.",
        "Pressione: Mid -> Output 1, Side -> Output 2.",
        "Pressione: Basse -> Output 1, Alte -> Output 2.",
        "Pressione: Transiente -> Output 1, Stazionario -> Output 2.",
        "Pressione: Picco -> Output 1, Stazionario -> Output 2.",
        "Regola la porzione di mix di Output 1 e Output 2.",
        "Scegli il tipo di filtro di separazione basse/alte.",
        "Scegli la pendenza del filtro di separazione basse/alte.",
        "Regola la frequenza di taglio del filtro di separazione basse/alte.",
        "Regola il bilanciamento della separazione transiente/stazionario. Minore è il bilanciamento, minore è il segnale transiente e maggiore è il segnale stazionario, e viceversa.",
        "Regola la forza della separazione transiente/stazionario. Minore è la forza, più morbida è la separazione.",
        "Regola l'hold della separazione transiente/stazionario. Maggiore è l'hold, più lento è il decadimento del segnale transiente.",
        "Regola la levigatezza dello spettro della separazione transiente/stazionario.",
        "Regola il bilanciamento della separazione picco/stazionario. Minore è il bilanciamento, minore è il segnale di picco e maggiore è il segnale stazionario, e viceversa.",
        "Regola l'attacco della separazione picco/stazionario.",
        "Regola l'hold della separazione picco/stazionario. Maggiore è l'hold, più lento è il decadimento del segnale di picco.",
        "Regola la levigatezza della separazione picco/stazionario. Maggiore è la levigatezza, maggiore è la dimensione della finestra RMS.",
        "Pressione: attiva l'analizzatore di spettro.",
        "Pressione: attiva l'analizzatore di magnitudine.",
        "Pressione: scambia Output 1 e Output 2.",
        "Rilascio: bypassa il plugin.",
        "Doppio clic: apri le impostazioni dell'interfaccia utente."
    };
}