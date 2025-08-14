// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "zlp_definitions.hpp"
#include "controller.hpp"

namespace zlp {
    template<typename FloatType>
    class ControllerAttach  final : private juce::AudioProcessorValueTreeState::Listener {
    public:
        explicit ControllerAttach(juce::AudioProcessor &processor,
                                  juce::AudioProcessorValueTreeState &parameters,
                                  zlp::Controller<FloatType> &controller);

        ~ControllerAttach() override;

    private:
        juce::AudioProcessor &p_ref_;
        juce::AudioProcessorValueTreeState &parameters_ref_;

        zlp::Controller<FloatType> &controller_ref_;

        static constexpr std::array kIDs{
            PSplitType::kID, PMix::kID, PSwap::kID,
            PLHFilterType::kID, PLHSlope::kID, PLHFreq::kID,
            PTSSeparation::kID, PTSBalance::kID, PTSHold::kID, PTSSmooth::kID,
            PPSAttack::kID, PPSBalance::kID, PPSHold::kID, PPSSmooth::kID
        };

        void parameterChanged(const juce::String &parameter_ID, float new_value) override;
    };
} // zlp
