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

namespace zlpanel::multilingual::zh_Hans {
    static constexpr std::array kTexts = {
        "按下：输入 -> Output 1, 静音 -> Output 2。",
        "按下：左声道 -> Output 1, 右声道 -> Output 2。",
        "按下：中声道 -> Output 1, 侧声道 -> Output 2。",
        "按下：低频 -> Output 1, 高频 -> Output 2。",
        "按下：瞬态 -> Output 1, 稳态 -> Output 2。",
        "按下：峰值 -> Output 1, 稳态 -> Output 2。",
        "调整 Output 1 和 Output 2 的混合比例。",
        "选择高低频分离的滤波器类型。",
        "选择高低频分离的滤波器斜率。",
        "调整高低频分离的滤波器截止频率。",
        "调整瞬态/稳态分离的平衡。平衡值越小，瞬态信号越少，稳态信号越多，反之亦然。",
        "调整瞬态/稳态分离的强度。强度越小，分离越柔和。",
        "调整瞬态/稳态分离的保持时间。保持时间越长，瞬态信号的衰减越慢。",
        "调整瞬态/稳态分离的频谱平滑度。",
        "调整峰值/稳态分离的平衡。平衡值越小，峰值信号越少，稳态信号越多，反之亦然。",
        "调整峰值/稳态分离的触发时间。",
        "调整峰值/稳态分离的保持时间。保持时间越长，峰值信号的衰减越慢。",
        "调整峰值/稳态分离的平滑度。平滑度越大，RMS 窗口大小越大。",
        "按下：打开频谱分析仪。",
        "按下：打开振幅分析仪。",
        "按下：交换 Output 1 和 Output 2。",
        "松开：旁通插件。",
        "双击：打开用户界面设置。"
    };
}
