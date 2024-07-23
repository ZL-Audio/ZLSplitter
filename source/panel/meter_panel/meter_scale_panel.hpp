//
// Created by Zishu Liu on 2/19/24.
//

#ifndef ZLWarm_METER_SCALE_PANEL_HPP
#define ZLWarm_METER_SCALE_PANEL_HPP

#include "../../gui/gui.hpp"

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
