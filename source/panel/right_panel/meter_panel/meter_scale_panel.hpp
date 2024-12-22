// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

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
    constexpr static bool ignoreFirst = true;
};

} // zlPanel

#endif //ZLWarm_METER_SCALE_PANEL_HPP
