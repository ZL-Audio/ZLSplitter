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

namespace zlpanel::multilingual::ja {
    static constexpr std::array kTexts = {
        "押す: 入力 -> Output 1、無音 -> Output 2。",
        "押す: Left -> Output 1、Right -> Output 2。",
        "押す: Mid -> Output 1、Side -> Output 2。",
        "押す: 低域 -> Output 1、高域 -> Output 2。",
        "押す: トランジェント -> Output 1、サステイン -> Output 2。",
        "押す: ピーク -> Output 1、サステイン -> Output 2。",
        "Output 1とOutput 2のミックス割合を調整します。",
        "低/高域分割フィルターのタイプを選択します。",
        "低/高域分割フィルターの傾斜（スロープ）を選択します。",
        "低/高域分割フィルターのカットオフ周波数を調整します。",
        "トランジェント/サステイン分割のバランスを調整します。バランスが小さいほどトランジェント信号が少なくなり、サステイン信号が多くなります。その逆も同様です。",
        "トランジェント/サステイン分割の強度を調整します。強度が小さいほど、分割がよりスムーズになります。",
        "トランジェント/サステイン分割のホールド時間を調整します。ホールド時間が長いほど、トランジェント信号の減衰が遅くなります。",
        "トランジェント/サステイン分割のスペクトル平滑度を調整します。",
        "ピーク/サステイン分割のバランスを調整します。バランスが小さいほどピーク信号が少なくなり、サステイン信号が多くなります。その逆も同様です。",
        "ピーク/サステイン分割のアタックタイムを調整します。",
        "ピーク/サステイン分割のホールド時間を調整します。ホールド時間が長いほど、ピーク信号の減衰が遅くなります。",
        "ピーク/サステイン分割の平滑度を調整します。平滑度が高いほど、RMSウィンドウサイズが大きくなります。",
        "押す: スペクトラムアナライザーをオンにします。",
        "押す: マグニチュードアナライザーをオンにします。",
        "押す: Output 1 と Output 2 を入れ替えます。",
        "離す: プラグインをバイパスします。",
        "ダブルクリック: UI設定を開きます。"
    };
}
