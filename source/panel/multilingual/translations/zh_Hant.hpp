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

namespace zlpanel::multilingual::zh_Hant {
    static constexpr std::array kTexts = {
        "按下：輸入 -> Output 1, 靜音 -> Output 2。",
        "按下：左聲道 -> Output 1, 右聲道 -> Output 2。",
        "按下：中聲道 -> Output 1, 側聲道 -> Output 2。",
        "按下：低頻 -> Output 1, 高頻 -> Output 2。",
        "按下：瞬態 -> Output 1, 穩態 -> Output 2。",
        "按下：峰值 -> Output 1, 穩態 -> Output 2。",
        "調整 Output 1 和 Output 2 的混合比例。",
        "選擇高低頻分離的濾波器類型。",
        "選擇高低頻分離的濾波器斜率。",
        "調整高低頻分離的濾波器截止頻率。",
        "調整瞬態/穩態分離的平衡。平衡值越小，瞬態訊號越少，穩態訊號越多，反之亦然。",
        "調整瞬態/穩態分離的強度。強度越小，分離越柔和。",
        "調整瞬態/穩態分離的保持時間。保持時間越長，瞬態訊號的衰減越慢。",
        "調整瞬態/穩態分離的頻譜平滑度。",
        "調整峰值/穩態分離的平衡。平衡值越小，峰值訊號越少，穩態訊號越多，反之亦然。",
        "調整峰值/穩態分離的觸發時間。",
        "調整峰值/穩態分離的保持時間。保持時間越長，峰值訊號的衰減越慢。",
        "調整峰值/穩態分離的平滑度。平滑度越大，RMS 窗口大小越大。",
        "按下：開啟頻譜分析儀。",
        "按下：開啟振幅分析儀。",
        "按下：交換 Output 1 和 Output 2。",
        "鬆開：旁通插件。",
        "雙擊：打開用戶界面設置。"
    };
}