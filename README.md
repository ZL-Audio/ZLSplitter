<p float="left">
  <img src="docs/zlaudio.svg" width="16.6%" />
  <img src="docs/logo.svg" width="7.5%" />
</p>

# ZLSplitter
![pluginval](<https://github.com/ZL-Audio/ZLSplit/actions/workflows/cmake_full_test.yml/badge.svg?branch=main>)
[![License](https://img.shields.io/badge/License-GPL3-blue.svg)](https://opensource.org/licenses/GPL-3.0)
[![Downloads](https://img.shields.io/github/downloads/ZL-Audio/ZLSplit/total)](https://somsubhra.github.io/github-release-stats/?username=ZL-Audio&repository=ZLSplit&page=1&per_page=30)

ZLSplitter is a splitter plugin.

<!-- <img src="Docs/screenshot.png" width=94%> -->
 
## Usage

See the wiki for details.

## Download

See the releases for the latest version. 

**Please NOTICE**:
- the installer has **NOT** been notarized/EV certificated on macOS/Windows
- the plugin has **NOT** been fully tested on DAWs

## Build from Source

0. `git clone` this repo

1. [Download CMAKE](https://cmake.org/download/) if you do not have it.

2. Populate all submodules by running `git submodule update --init` in your repository directory.

3. Install [Boost](https://www.boost.org/).

4. Follow the [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md) to build the source.

## License

ZLEqualizer is licensed under GPLv3, as found in the [LICENSE.md](LICENSE.md) file.

Copyright (c) 2024 - [zsliu98](https://github.com/zsliu98)

JUCE framework from [JUCE](https://github.com/juce-framework/JUCE)

JUCE template from [pamplejuce](https://github.com/sudara/pamplejuce)

[fft-juce](https://github.com/hollance/fft-juce) from [
Matthijs Hollemans](https://github.com/hollance)

Font from CMU Open Sans, Font Awesome and MiSans.

## References

Redmon, Nigel. *Cascading filters*. (2016).

Fitzgerald, Derry. *Harmonic/percussive separation using median filtering*. (2010).