# ZLSplitter 修改说明

## 1. TS/PSSplitter 前置 Flat-Tilt EQ

为 **Transient-Steady（TS）** 与 **Peak-Steady（PS）** 两种分离模式增加前置 Tilt Shelf EQ，可在瞬态/峰值分离前对输入信号做频率倾斜预处理，引导分离器关注特定频段。

### 实现方式
- **参数层**：在 `source/zlp/zlp_definitions.hpp` 新增 `PTiltFreq`（20Hz–20kHz，默认 1kHz，对数中点分布）与 `PTiltGain`（±12dB，默认 0dB）两个 `AudioProcessorValueTreeState` 参数，纳入 DAW 自动化与状态持久化。
- **DSP 层**：`source/zlp/controller.hpp` 引入 `zldsp::filter::IIR<FloatType, 2> tilt_filter_`。该实例直接复用项目已有的 IIR 二阶级联框架（`source/dsp/filter/iir_filter/single_iir_filter.hpp`），滤波器类型设为 `kTiltShelf`，系数由 `MartinCoeff::get2TiltShelf` 计算，无需新增任何 DSP 算法模块。
- **信号流**：在 `source/zlp/controller.cpp` 的 `process()` 中，仅对 `kTSteady` 与 `kPSteady` 分支先执行 `tilt_filter_.process(in_buffer, num_samples)`（就地修改），再送入对应 splitter；其他模式不受影响。
- **参数映射**：`source/zlp/controller_attach.cpp` 将 `PTiltFreq` 直接映射为滤波器频率；`PTiltGain` 的 dB 值通过 `pow(10, db * 0.1)` 转换为功率增益后写入滤波器。
- **UI 层**：`source/panel/control_panel/ts_pop_panel.hpp|cpp` 与 `ps_pop_panel.hpp|cpp` 各新增两组 `CompactLinearSlider` + `SliderAttachment`，与 Balance、Strength 等原有控件并列；`getIdealHeight` 相应增加两行布局高度。
- **多语言**：`source/panel/multilingual/tooltip_labels.hpp` 新增 `kTiltFreq`、`kTiltGain` 枚举，7 个语言翻译文件同步补充 tooltip 文本。

### 涉及文件
```
source/zlp/zlp_definitions.hpp
source/zlp/controller.hpp
source/zlp/controller.cpp
source/zlp/controller_attach.hpp
source/zlp/controller_attach.cpp
source/panel/control_panel/ts_pop_panel.hpp
source/panel/control_panel/ts_pop_panel.cpp
source/panel/control_panel/ps_pop_panel.hpp
source/panel/control_panel/ps_pop_panel.cpp
source/panel/multilingual/tooltip_labels.hpp
source/panel/multilingual/translations/* (7 files)
```

---

## 2. 切换 Splitter 模式时自动重置专属参数

### 背景
TS/PS 模式的 Tilt 参数、Mix、Balance 等属于模式专属或高度相关的调节项。用户在切换分离模式时，旧模式的参数残留会导致新模式的初始状态不可预期。

### 实现方式
- **参数重置**：在 `source/zlp/controller_attach.cpp` 的 `PSplitType` 参数变化回调中，通过 `getParameterAsValue(id).setValue(kDefaultV)` 将以下参数重置为默认值：
  - **Mix** → `0`
  - **TS Balance** → `0`
  - **PS Balance** → `0`
  - **Tilt Gain** → `0dB`（仅在进入 TS/PS 时触发）
  - **Tilt Freq** → `1000Hz`（仅在进入 TS/PS 时触发）
- **DSP 状态同步**：`source/zlp/controller.cpp` 的 `prepareBuffer()` 在检测到 `split_type` 变化时调用 `tilt_filter_.reset()`，清空 IIR 历史状态，避免模式切换瞬间产生咔哒声或状态污染。

> **注意**：`getParameterAsValue().setValue()` 直接操作底层 `ValueTree` 属性，该属性存储的是 **原始值（unnormalised）**，因此直接传入 `kDefaultV` 即可。JUCE 内部会自行完成 `normalise → setValueNotifyingHost → denormalise` 的转换链路。若误传入 `NormalisableRange::convertTo0to1` 得到的 normalized 值，会导致二次映射失真（例如 Balance 被映射到 0.5、Tilt Freq 被映射到 20Hz 等）。

### 涉及文件
```
source/zlp/controller.cpp
source/zlp/controller_attach.cpp
```

---

## 3. Tooltip 悬停响应提速

### 实现方式
- 将 `source/gui/tooltip/tooltip_window.hpp` 中 `juce::TooltipWindow` 的默认悬停等待时间从 **700ms** 缩短至 **200ms**，使鼠标悬停到控件上时提示框更快出现。

### 涉及文件
```
source/gui/tooltip/tooltip_window.hpp
```
