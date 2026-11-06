<p align="center">
  <img src="docs/zlaudio.svg" width="7.5%" hspace="6.068%"/>
  <img src="docs/logo.svg" width="7.5%" hspace="6.068%"/>
</p>

# ZL Splitter
![pluginval](<https://github.com/ZL-Audio/ZLSplitter/actions/workflows/cmake_full_test.yml/badge.svg?branch=main>)
[![License](https://img.shields.io/badge/License-AGPLv3-blue.svg)](https://opensource.org/license/agpl-v3)
[![Downloads](https://img.shields.io/github/downloads/ZL-Audio/ZLSplitter/total)](https://somsubhra.github.io/github-release-stats/?username=ZL-Audio&repository=ZLSplitter&page=1&per_page=30)

ZL Splitter is a splitter plugin.

<img src="https://drive.google.com/uc?export=view&id=1agmMFD5BVylbJOr5ZJmP8dof7LkCXlhx" style="width:750px; max-width: 100%; height: auto" />

## Build from Source

### Install Dependencies

Please make sure `Clang` (`AppleClang 16+` or `LLVM/Clang 17+`), `cmake 3.25+`, `ninja` are installed and configured on your OS.

On Linux, you can install the remaining dependencies with the following command:

```console
sudo apt-get update && sudo apt install libasound2-dev libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libfreetype-dev libfontconfig1-dev libxi-dev
```

### Clone and Build

Once you have set up the environment, you can clone the ZL Splitter code, populate all submodules, then configure & build the code. populate all submodules, then configure & build the code. Please set:

- the variable `ZL_JUCE_FORMATS` as a list of plugin formats that you want, e.g., `"VST3;LV2"`
    - AAX plug-ins need to be digitally signed using PACE Anti-Piracy's signing tools before they will run in commercially available versions of Pro Tools.
- the variable `ZL_HWY_STATIC_TARGET` as the SIMD target
    - If you are on x86-64 and your CPU supports SSE2, set `ZL_HWY_STATIC_TARGET=SSE2`.
    - If you are on x86-64 and your CPU supports SSE2/SSE4, set `ZL_HWY_STATIC_TARGET=SSE4`.
    - If you are on x86-64 and your CPU supports SSE2/SSE4/AVX2, set `ZL_HWY_STATIC_TARGET=AVX2`.
    - If you are on arm64, set `ZL_HWY_STATIC_TARGET=NEON`.
- If there are multiple compilers on your OS, you may need to pass extra flags to maker sure that cmake uses `LLVM/Clang`.
    - On Linux, you may pass `-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++`.
    - On Windows, you may pass `-DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl`.

```console
git clone https://github.com/ZL-Audio/ZLSplitter
cd ZLSplitter
git submodule update --init --recursive
cmake -B Builds -G Ninja -DCMAKE_BUILD_TYPE=Release -DZL_JUCE_FORMATS="VST3;LV2" -DZL_HWY_STATIC_TARGET="NEON" .
cmake --build Builds --config Release
```

After building, the plugins should have been copied to the corresponding folders. If you want to disable the copy process, you can pass `-DZL_JUCE_COPY_PLUGIN=FALSE`, find the binary folders under `Builds/ZLSplitter_artefacts/Release` and copy them manually.

## AI Transparency

Since November 2026, LLMs have assisted across various stages of this project's development. All code, documentation, and design choices are either authored or reviewed by the maintainer, who takes full responsibility for the quality, safety, and licensing of the final output.

## License

ZL Splitter is licensed under AGPLv3, as found in the [LICENSE.md](LICENSE.md) file. However, the [logo of ZL Audio](assets/zlaudio.svg) and the [logo of ZL Splitter](assets/logo.svg) are not covered by this license.

Copyright (c) 2024 - [zsliu98](https://github.com/zsliu98)

JUCE framework from [JUCE](https://github.com/juce-framework/JUCE)

JUCE template from [pamplejuce](https://github.com/sudara/pamplejuce)

[Highway](https://github.com/google/highway) by [Google](https://github.com/google)

[zldsp_fft](https://github.com/ZL-Audio/zldsp_fft) by [ZL Audio](https://github.com/ZL-Audio)

[Material Symbols](https://github.com/google/material-design-icons) by [Google](https://github.com/google)

[inter](https://github.com/rsms/inter) by [The Inter Project Authors](https://github.com/rsms/inter)

## References

Yuriy Ivantsov. *On the Ideal Bilinear and Biquadratic Digital Filter*. (2025).

Vicanek, Martin. *A New Reverse IIR Filtering Algorithm*. (2022).

Redmon, Nigel. *Cascading filters*. (2016).

Fitzgerald, Derry. *Harmonic/percussive separation using median filtering*. (2010).

Cleve Moler. [*Makima Piecewise Cubic Interpolation*](https://blogs.mathworks.com/cleve/2019/04/29/makima-piecewise-cubic-interpolation/). MathWorks Blogs. (2019).
