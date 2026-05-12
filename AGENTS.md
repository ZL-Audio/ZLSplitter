# ZL Splitter - Agent 指南

本文档面向在 ZL Splitter 项目上工作的 AI 编程 Agent，汇总了你需要了解的架构、构建系统、代码规范和工作流程。

## 项目概览

**ZL Splitter** 是一款多格式音频插件，它使用多种算法之一将立体声输入拆分为两路立体声输出：

-**Left-Right (L/R)** - 基于声像的左/右分离

-**Mid-Side (M/S)** - 中/侧解码

-**Low-High** - 使用分频滤波器（SVF 或 FIR）按频率拆分

-**Transient-Steady** - 基于 FFT 的中值滤波分离（基于 Fitzgerald, *Harmonic/percussive separation using median filtering*, 2010）

-**Peak-Steady** - 包络跟随器 / 压缩器风格的峰值/稳态分离

-**None** - 直通

该插件基于 **JUCE** 框架构建，使用 **KFR** 进行 SIMD/向量化 DSP 运算，采用 **AGPLv3** 许可证。

-**作者:** zsliu98 / ZL Audio

-**仓库:** https://github.com/ZL-Audio/ZLSplitter

-**语言:** C++20

-**构建系统:** CMake (最低 3.24.1) + Ninja

-**模板基础:** [Pamplejuce](https://github.com/sudara/pamplejuce)

## 技术栈

| 组件 | 技术 |

|-----------|------------|

| 插件框架 | JUCE (git 子模块位于 `JUCE/`) |

| DSP / SIMD | KFR (git 子模块位于 `kfr/`) |

| 构建系统 | CMake 3.24.1+, Ninja |

| 所需编译器 | Clang / AppleClang 16+ 或 LLVM/Clang 17+ |

| 可选加速 | Intel IPP (`IPP_DIR` 环境变量) |

| 代码格式化工具 | `clang-format` (配置位于 `.clang-format`) |

| CI / CD | GitHub Actions |

| 插件验证 | [pluginval](https://github.com/Tracktion/pluginval) |

### 支持的插件格式

默认情况下：

-**macOS:** VST3, AU, Standalone

-**Linux / Windows:** VST3, LV2, Standalone

AAX 可选支持，但需要 PACE 签名工具才能在 Pro Tools 中运行。格式列表由 CMake 变量 `ZL_JUCE_FORMATS` 控制。

## 仓库结构

```

CMakeLists.txt              - 主 CMake 配置

build.sh                    - 便捷构建脚本（Release, VST3+Standalone）

.clang-format               - 代码风格配置

.gitmodules                 - JUCE 和 KFR 子模块

source/                     - 所有插件源代码

  PluginProcessor.cpp/hpp   - JUCE AudioProcessor 入口点

  PluginEditor.cpp/hpp      - JUCE AudioProcessorEditor 入口点

  dsp/                      - DSP 算法与工具

    splitter/               - Splitter 实现 (LR, MS, LH, TS, PS)

    filter/                 - IIR, FIR, SVF, reverse IIR, ideal 滤波器

    analyzer/               - FFT, magnitude, 和 wave 分析器

    compressor/             - 包络跟随器 / 压缩器构建块

    fft/                    - KFR FFT 引擎封装

    delay/                  - 延迟线实现

    container/              - 循环缓冲区, FIFOs

    vector/                 - KFR 向量导入辅助工具

    chore/                  - 小型工具（平滑值、分贝转换）

  gui/                      - 可复用的 JUCE GUI 组件

    button/                 - 自定义按钮

    slider/                 - 自定义滑块

    combobox/               - 自定义下拉框

    colour_selector/        - 色图 / 不透明度选择器

    tooltip/                - 工具提示系统

    attachment/             - 参数到组件的绑定器

    interface_definitions.hpp  - UI 常量与外观辅助工具

  panel/                    - 插件专属 UI 面板

    main_panel.hpp          - 根布局组件

    curve_panel/            - 分析器显示面板 (FFT, MAG, WAV)

    control_panel/          - 参数控制弹窗

    top_panel/              - Logo, 图例, 选择栏

    analyzer_setting_panel/ - 分析器设置

    ui_setting_panel/       - UI 设置（颜色、字体、行为）

    multilingual/           - 工具提示翻译 (EN, DE, ES, IT, JA, ZH_Hans, ZH_Hant)

    helper/                 - 面板辅助工具（刷新处理器、路径简化器、常量）

  state/                    - 插件状态管理

    state_definitions.hpp   - 非自动化 UI 参数

    property.cpp/hpp        - UI 属性管理

    dummy_processor.hpp     - 仅用于编辑器状态的占位处理器

  zlp/                      - 主插件控制器 / 桥接层

    zlp_definitions.hpp     - 可自动化插件参数

    controller.hpp          - DSP 控制器（编排 splitters + analyzers）

    controller_attach.hpp   - 参数到控制器的绑定器

cmake-includes/             - 模块化 CMake 脚本

  Assets.cmake              - 将 assets/ 嵌入为二进制数据

  GitVersion.cmake          - 从 git 标签推导版本号

  PamplejuceVersion.cmake   - 版本号辅助工具

  PamplejuceMacOS.cmake     - macOS 通用二进制配置

  JUCEDefaults.cmake        - JUCE 编译默认值

  Sanitizer.cmake           - Address / Thread sanitizer 选项

  SharedCodeDefaults.cmake  - SharedCode 目标默认值

  Tests.cmake               - Catch2 测试配置（当前未包含）

  Benchmarks.cmake          - 基准测试目标（当前未包含）

  XcodePrettify.cmake       - Xcode 项目组织

  GitHubENV.cmake           - 导出构建信息到 .env 供 CI 使用

assets/                     - 字体、图标、Logo（嵌入为 BinaryData）

packaging/                  - 安装包脚本（macOS .pkg, Windows .msi, Linux .zip）

.github/workflows/          - CI/CD 定义

  cmake_full_test.yml       - push/PR 时构建 + 测试 + 打包

  cmake_heavy_test.yml      - 扩展 pluginval 压力测试（手动触发）

  cmake_release.yml         - 多架构发布构建 + GitHub 发布

  cmake_nightly_release.yml - 夜间构建推送至 ZLNightlyRelease 仓库

  sanitizer.yml             - Address / Thread sanitizer 运行（手动触发）

  sync.yml                  - 子模块同步

docs/                       - Logo 和文档资源

```

## 构建说明

### 前置要求

-**CMake** 3.25+

-**Ninja**

-**Clang** (macOS 上 AppleClang 16+, Linux/Windows 上 LLVM/Clang 17+)

- Windows (Intel 芯片): 可选安装 Intel IPP 并设置 `IPP_DIR`

**Linux 系统依赖：**

```bash

sudoapt-getupdate && sudoaptinstalllibasound2-devlibx11-devlibxinerama-devlibxext-devlibfreetype-devlibfontconfig1-devlibwebkit2gtk-4.1-devlibglu1-mesa-dev

```

### 克隆与构建

```bash

gitclonehttps://github.com/ZL-Audio/ZLSplitter

cdZLSplitter

gitsubmoduleupdate--init--recursive

```

**Release 构建（示例：VST3 + LV2）：**

```bash

cmake-BBuilds-GNinja-DCMAKE_BUILD_TYPE=Release-DKFR_ENABLE_MULTIARCH=OFF-DZL_JUCE_FORMATS="VST3;LV2".

cmake--buildBuilds--configRelease

```

### 重要的 CMake 选项

| 选项 | 说明 | 默认值 |

|--------|-------------|---------|

| `ZL_JUCE_FORMATS` | 分号分隔的插件格式列表 | 取决于操作系统 |

| `ZL_JUCE_COPY_PLUGIN` | 构建后复制插件到系统目录 | `TRUE` |

| `KFR_ENABLE_MULTIARCH` | 启用 KFR 多架构构建 | 随 CI 矩阵变化 |

| `KFR_ARCHS` | KFR 目标架构（例如 `sse2;avx2`） | - |

| `WITH_ADDRESS_SANITIZER` | 启用 AddressSanitizer | `OFF` |

| `WITH_THREAD_SANITIZER` | 启用 ThreadSanitizer | `OFF` |

**编译器提示：**

- Linux: 添加 `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`
- Windows: 添加 `-DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl`

构建完成后，插件会自动复制到系统插件文件夹（除非 `ZL_JUCE_COPY_PLUGIN=FALSE`）。二进制文件也可在 `Builds/ZLSplitter_artefacts/Release/` 下找到。

## 代码组织与架构

### 命名空间约定

| 命名空间 | 用途 |

|-----------|---------|

| `zldsp` | DSP 算法与工具 |

| `zldsp::splitter` | 音频 Splitter (LR, MS, LH, TS, PS) |

| `zldsp::analyzer` | 分析器 (FFT, magnitude, wave) |

| `zldsp::filter` | 滤波器实现 |

| `zldsp::compressor` | 压缩器 / 包络跟随器模块 |

| `zldsp::delay` | 延迟线 |

| `zldsp::container` | 数据结构（循环缓冲区、FIFOs） |

| `zldsp::chore` | 小型工具（平滑值、分贝转换） |

| `zlgui` | 可复用 GUI 组件 |

| `zlpanel` | 插件专属面板布局 |

| `multilingual` | 工具提示翻译与语言辅助工具 |

| `zlstate` | 插件状态 / 非自动化参数 |

| `zlp` | 主插件参数与 DSP 控制器 |

### 关键架构模式

1.**SharedCode Target:** 所有源文件都被编译到一个名为 `SharedCode` 的 `INTERFACE` 库中。该库被链接到 JUCE 插件目标中。此模式（来自 Pamplejuce）使得测试目标可以复用代码而不会导致 ODR 违规。

2.**基于模板的 DSP:** 大多数 DSP 类都是仅头文件的模板，由 `FloatType` 参数化（通常为 `float` 或 `double`）。处理器同时支持单精度和双精度。

3.**原子参数通信:**`Controller` 类使用 `std::atomic` 标志，以线程安全的方式将参数变更从消息线程传递到音频线程。

4.**基于 FIFO 的分析器:** 分析器系统使用无锁 FIFO 将音频数据从音频线程传输到 GUI 线程进行可视化。

5.**Attachment 模式:** JUCE `AudioProcessorValueTreeState` 参数通过 `source/gui/attachment/` 中的自定义 attachment 类绑定到 UI 组件。

### 入口点

-**`PluginProcessor`** (`source/PluginProcessor.hpp`): JUCE `AudioProcessor` 子类。持有 `zlp::Controller<double>` 和 `zlp::ControllerAttach<double>`。支持 `float` 和 `double` 精度处理音频。

-**`PluginEditor`** (`source/PluginEditor.hpp`): JUCE `AudioProcessorEditor` 子类。持有 `zlpanel::MainPanel` 并通过 `zlstate::Property` 管理 UI 状态。

## 代码风格指南

本项目使用 **`.clang-format`**，关键设置如下：

-**基础风格:** LLVM

-**列宽限制:** 120

-**缩进宽度:** 4 个空格

-**制表符宽度:** 4

-**命名空间缩进:** All

-**指针对齐:** 靠左 (`float* ptr`)

-**始终打断模板声明:** 是

-**自定义大括号换行:**`BeforeCatch: true`, `BeforeWhile: true`

-**构造函数初始化列表换行:** 冒号之后

-**最大空行数:** 2

-**Include 分类:** 系统头文件优先 (`<...>`)，然后是本地头文件 (`"..."`)，最后是其他

**提交前请对所有修改过的文件运行 `clang-format`。**

### 命名约定（代码库中实际使用的）

-**类 / 结构体:** PascalCase (`TSSplitter`, `PluginProcessor`)

-**成员变量:** 尾部下划线 (`sample_rate_`, `mix_`)

-**模板参数:**`FloatType`, `kNum`

-**常量 / 枚举:**`kPascalCase` (`kBandNum`, `kVersionHint`)

-**命名空间:** 小写加下划线 (`zldsp`, `zldsp::splitter`)

-**文件名:** snake_case，扩展名为 `.hpp` / `.cpp`

### 文件组织

- 头文件使用 `#pragma once`
- 每个源文件以 AGPLv3 版权声明头开头
- UI 相关的 `.cpp` 文件与同目录下的 `.hpp` 头文件配对
- DSP 代码大多是仅头文件的 `.hpp` 模板

## 测试策略

### 当前状态

-**目前尚无单元测试。**`tests/` 目录不存在，`cmake-includes/Tests.cmake` (Catch2 配置) 也**未包含**在主 `CMakeLists.txt` 中。

-`cmake-includes/Benchmarks.cmake` 同样被注释掉了。

### CI 验证

本项目严重依赖 **[pluginval](https://github.com/Tracktion/pluginval)** 进行集成测试：

-**严格等级:** 10 (最高)

-**禁用测试:**`Plugin state restoration`

-**重复次数:** 2 (完整测试), 10 (压力测试 / sanitizer)

-**超时:** 压力测试最长可达 2 小时

### 工作流

| 工作流 | 触发条件 | 用途 |

|----------|---------|---------|

| `cmake_full_test.yml` | push 到 `main`/`develop`，向 `main` 提 PR | 构建, CTest, pluginval, 打包 |

| `cmake_heavy_test.yml` | 手动触发 | 扩展 pluginval 压力测试 |

| `sanitizer.yml` | 手动触发 | AddressSanitizer + ThreadSanitizer + pluginval |

| `cmake_release.yml` | 手动触发 (tags) | 多架构发布构建 + GitHub 发布 |

| `cmake_nightly_release.yml` | 手动触发 | 夜间构建推送至 `ZLNightlyRelease` 仓库 |

### 添加单元测试

如果你要添加测试：

1. 在项目根目录创建 `tests/` 目录。
2. 在 `CMakeLists.txt` 中包含 `cmake-includes/Tests.cmake`。
3. 使用 **Catch2 v3** (`Tests.cmake` 中已配置)。
4. 测试会通过 `catch_discover_tests(Tests)` 自动发现。

## 部署与打包

### macOS

-**代码签名:** 单个插件包使用 `DEVELOPER_ID_APPLICATION` 签名。

-**安装包:** 通过 `packaging/packager_macOS.py` 生成 `.pkg`。

-**公证:** 可选（需要 `NOTARIZATION_USERNAME`, `NOTARIZATION_PASSWORD`, `TEAM_ID`）。

-**分发:** 使用 `hdiutil` 创建 `.dmg`。

### Windows

-**安装包:** 通过 WiX v5 (`packaging/packager_Windows_WiX.py`) 生成 `.msi`。

-**架构:** 支持 x64 和 ARM64。

### Linux

-**分发:** artefacts 目录的 `.zip` 压缩包。

-**架构:** 支持 x86 (SSE2/AVX2) 和 ARM64。

## 安全考量

-**许可证:** AGPLv3。任何分发的修改也必须采用 AGPLv3。

-**代码签名:** macOS 构建期望使用 GitHub secrets 中的签名证书。如果缺少 secrets，仍会生成未签名构建。

-**AAX 签名:** AAX 插件需要 PACE Anti-Piracy 签名工具，没有它们则无法在商业版 Pro Tools 中使用。

-**Sanitizers:** 提供 AddressSanitizer 和 ThreadSanitizer 配置，用于调试内存和线程问题。

## 开发约定

### 参数

该插件使用**两个独立的 `AudioProcessorValueTreeState` 实例**：

1.**`parameters_`** (`PluginProcessor`) - 可自动化的 DSP 参数，定义在 `source/zlp/zlp_definitions.hpp` 中：

- 拆分类型、混合量、交换、旁通
- Low-High 滤波器类型、斜率、频率
- Transient-Steady 和 Peak-Steady 控制

2.**`na_parameters_`** (non-automatable) 和 **`state_`** (编辑器状态) - 仅 UI 使用的参数，定义在 `source/state/state_definitions.hpp` 中：

- 窗口大小、字体设置、色图
- 分析器显示设置（FFT 倾斜、速度、平滑等）
- 工具提示语言

### 添加新参数

1. 在 `zlp_definitions.hpp`（可自动化）或 `state_definitions.hpp`（非自动化）中定义新的参数类。
2. 继承自 `FloatParameters<T>`、`BoolParameters<T>` 或 `ChoiceParameters<T>`。
3. 将其添加到相应的 `getParameterLayout()` 或 `getNAParameterLayout()` 函数中。
4. 将其绑定到控制器或 UI 组件。

### 添加新 Splitter

1. 在 `source/dsp/splitter/<name>_splitter/` 下实现 splitter，置于 `zldsp::splitter` 命名空间中。
2. 从 `source/dsp/splitter/splitter.hpp` 中暴露它。
3. 在 `source/zlp/controller.hpp` 的 `zlp::Controller` 中添加它。
4. 在 `zlp_definitions.hpp` 中添加对应的 `SplitType` 枚举值。
5. 在 `source/panel/control_panel/` 下的相应面板中添加 UI 控件。

## 有用参考

- JUCE CMake API: https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
- KFR Documentation: https://www.kfrlib.com/
- Pamplejuce: https://github.com/sudara/pamplejuce
- pluginval: https://github.com/Tracktion/pluginval

---

*最后更新: 2026-05-11*
