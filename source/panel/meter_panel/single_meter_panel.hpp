// Copyright (C) 2024 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#ifndef ZLWarm_SINGLE_METER_PANEL_HPP
#define ZLWarm_SINGLE_METER_PANEL_HPP

#include "../../dsp/dsp.hpp"
#include "meter_scale_panel.hpp"
#include "../../gui/gui.hpp"

namespace zlPanel {

class SingleMeterPanel final : public juce::Component {
public:
    explicit SingleMeterPanel(zlMeter::SingleMeter<double> &meter, zlInterface::UIBase &base);

    ~SingleMeterPanel() override;

    void paint(juce::Graphics &g) override;

    void resized() override;

    void mouseDown(const juce::MouseEvent &event) override;

private:
    zlMeter::SingleMeter<double> &m;
    zlInterface::UIBase &uiBase;
    constexpr static float maxDB = 0.f, minDB = -60.f;
    constexpr static size_t numScales = 5;
    constexpr static float labelPortion = .075f, paddingPortion = .2f;

    MeterScalePanel scalePanel;

    juce::Path path;
};

} // zlPanel

#endif //ZLWarm_SINGLE_METER_PANEL_HPP
