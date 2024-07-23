// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLWarm_METER_SCALE_PANEL_HPP
#define ZLWarm_METER_SCALE_PANEL_HPP

#include "../../../gui/gui.hpp"

namespace zlPanel {

class MeterScalePanel final : public juce::Component {
public:
    explicit MeterScalePanel(zlInterface::UIBase &base);

    void paint(juce::Graphics &g) override;

private:
    zlInterface::UIBase &uiBase;
    constexpr static float maxDB = 0.f, minDB = -60.f;
    constexpr static size_t numScales = 5;
    constexpr static float labelPortion = .075f;
    constexpr static bool ignoreFirst = true;
};

} // zlPanel

#endif //ZLWarm_METER_SCALE_PANEL_HPP
