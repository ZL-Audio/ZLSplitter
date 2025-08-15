// Copyright (C) 2025 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "controller_attach.hpp"

namespace zlp {
    template<typename FloatType>
    ControllerAttach<FloatType>::ControllerAttach(juce::AudioProcessor &processor,
                                                  juce::AudioProcessorValueTreeState &parameters,
                                                  Controller<FloatType> &controller)
        : p_ref_(processor), parameters_ref_(parameters),
          controller_ref_(controller) {
        for (auto &ID: kIDs) {
            parameters_ref_.addParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    ControllerAttach<FloatType>::~ControllerAttach() {
        for (auto &ID: kIDs) {
            parameters_ref_.removeParameterListener(ID, this);
        }
    }

    template<typename FloatType>
    void ControllerAttach<FloatType>::parameterChanged(const juce::String &parameter_ID, const float new_value) {
        if (parameter_ID == zlp::PSplitType::kID) {
            controller_ref_.setSplitType(static_cast<zlp::PSplitType::SplitType>(std::round(new_value)));
        } else if (parameter_ID == zlp::PMix::kID) {
            controller_ref_.setMix(static_cast<FloatType>(new_value * 0.005f));
        } else if (parameter_ID == zlp::PLHFreq::kID) {
            controller_ref_.getLHSplitter().setFreq(static_cast<double>(new_value));
        } else if (parameter_ID == zlp::PLHSlope::kID) {
            controller_ref_.getLHSplitter().setOrder(
                zlp::PLHSlope::kOrders[static_cast<size_t>(std::round(new_value))]);
        }
    }

    template class ControllerAttach<float>;

    template class ControllerAttach<double>;
}
