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

namespace zlpanel::multilingual::es {
    static constexpr std::array kTexts = {
        "Pulsar: Entrada -> Output 1, Silencio -> Output 2.",
        "Pulsar: Left -> Output 1, Right -> Output 2.",
        "Pulsar: Mid -> Output 1, Side -> Output 2.",
        "Pulsar: Graves -> Output 1, Agudos -> Output 2.",
        "Pulsar: Transitorio -> Output 1, Constante -> Output 2.",
        "Pulsar: Pico -> Output 1, Constante -> Output 2.",
        "Ajustar la porción de mezcla de Output 1 y Output 2.",
        "Seleccionar el tipo de filtro de división de graves/agudos.",
        "Seleccionar la pendiente del filtro de división de graves/agudos.",
        "Ajustar la frecuencia de corte del filtro de división de graves/agudos.",
        "Ajustar el balance de la división transitorio/constante. Cuanto menor sea el balance, menor será la señal transitoria y mayor la señal constante, y viceversa.",
        "Ajustar la fuerza de la división transitorio/constante. Cuanto menor sea la fuerza, más suave será la división.",
        "Ajustar el tiempo de espera de la división transitorio/constante. Cuanto mayor sea el tiempo, más lenta será la caída de la señal transitoria.",
        "Ajustar la suavidad del espectro de la división transitorio/constante.",
        "Ajustar el balance de la división pico/constante. Cuanto menor sea el balance, menor será la señal de pico y mayor la señal constante, y viceversa.",
        "Ajustar el ataque de la división pico/constante.",
        "Ajustar el tiempo de espera de la división pico/constante. Cuanto mayor sea el tiempo, más lenta será la caída de la señal de pico.",
        "Ajustar la suavidad de la división pico/constante. Cuanto mayor sea la suavidad, mayor será el tamaño de la ventana RMS.",
        "Pulsar: activar el analizador de espectro.",
        "Pulsar: activar el analizador de magnitud.",
        "Pulsar: intercambiar Output 1 y Output 2.",
        "Soltar: anular el plugin.",
        "Doble clic: abrir la configuración de la interfaz de usuario."
    };
}