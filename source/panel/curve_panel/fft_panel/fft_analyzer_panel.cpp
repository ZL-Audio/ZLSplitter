// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "fft_analyzer_panel.hpp"
#include "../../../zlp/sample_rate_helper.hpp"

namespace zlpanel {
    FFTAnalyzerPanel::FFTAnalyzerPanel(PluginProcessor& p, zlgui::UIState& base) :
        p_ref_(p),
        base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)),
        fft_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTMinDB::kID)),
        fft_smooth_oct_value_idx_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTSmoothOCTValue::kID)),
        fft_smooth_erb_value_idx_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTSmoothERBValue::kID)),
        fft_smooth_type_idx_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTSmoothType::kID)),
        fft_speed_idx_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTSpeed::kID)),
        fft_tilt_idx_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTTilt::kID)) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::Controller<double>::kAnalyzerPointNum) * 3 + 1;
        for (auto& path : {&out_path1_, &out_path2_, &next_out_path1_, &next_out_path2_}) {
            path->preallocateSpace(preallocateSpace);
        }
        for (auto& receiver : receivers_) {
            receiver.setON(true);
        }
        setInterceptsMouseClicks(false, false);

        lookAndFeelChanged();
    }

    FFTAnalyzerPanel::~FFTAnalyzerPanel() = default;

    void FFTAnalyzerPanel::paint(juce::Graphics& g) {
        const std::unique_lock lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto thickness = base_.gui.getFontSize() * .2f * base_.getFFTCurveThickness();
        if (static_cast<zlp::PSplitType::SplitType>(
            std::round(split_type_ref_.load(std::memory_order_relaxed))) == zlp::PSplitType::kNone) {
            g.setColour(base_.gui.getTextColour());
            g.strokePath(out_path1_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        } else {
            const auto swap = swap_ref_.load(std::memory_order::relaxed) > .5f;
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput2Colour));
            g.strokePath(swap ? out_path1_ : out_path2_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
            g.setColour(base_.getColourByIdx(zlgui::ColourIdx::kOutput1Colour));
            g.strokePath(swap ? out_path2_ : out_path1_,
                         juce::PathStrokeType(thickness,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
        }
    }

    void FFTAnalyzerPanel::resized() {
        const auto bound = getLocalBounds().toFloat();
        atomic_bound_.store(bound);
    }

    void FFTAnalyzerPanel::run() {
        const auto bound = atomic_bound_.load();
        if (bound.isEmpty()) { return; }
        const auto min_db = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(
            fft_min_db_ref_.load(std::memory_order::relaxed)))] / 6.f * 7.f;
        if (std::abs(min_db - c_fft_min_db_) > .1f) {
            c_fft_min_db_ = min_db;
            to_update_decay_.store(true, std::memory_order::relaxed);
        }
        bool to_update_xs{false};
        bool to_update_smooth{false};
        const auto high_quality = base_.getFFTQuality() == zlstate::PFFTQuality::kHigh;
        const auto first_resolution = high_quality ? kLowResolution : kMiddleResolution;
        const auto end_resolution = high_quality ? kNumResolutions : kMiddleResolution + 1;
        double sample_rate;
        {
            auto& sender{p_ref_.getController().getAnalyzerSender()};
            const std::unique_lock lock{sender.getLock(), std::try_to_lock};
            if (!lock.owns_lock()) { return; }
            sample_rate = sender.getSampleRate();
            if (!std::isfinite(sample_rate) || sample_rate <= 20.0 || sender.getMaxNumSamples() == 0) { return; }

            if (generation_ != sender.getGeneration() || std::abs(c_sample_rate_ - sample_rate) > 0.1 ||
                high_quality != c_high_quality_) {
                generation_ = sender.getGeneration();
                c_sample_rate_ = sample_rate;
                c_high_quality_ = high_quality;
                to_update_tilt_.store(true, std::memory_order::relaxed);

                const auto middle_fft_order = static_cast<int>(zlp::getScaledOrder(sample_rate, 12));
                const std::array fft_orders{
                    middle_fft_order + 2, middle_fft_order, middle_fft_order - 2
                };
                for (size_t i = first_resolution; i < end_resolution; ++i) {
                    processors_[i].prepare(fft_orders[i]);
                }
                // equalize the expected white-noise power of the Hann windows
                const auto reference_window_power = processors_[kMiddleResolution].getWindowSqrSum();
                for (size_t i = first_resolution; i < end_resolution; ++i) {
                    noise_power_scales_[i] = static_cast<float>(
                        reference_window_power / processors_[i].getWindowSqrSum());
                }
                history_size_ = static_cast<int>(processors_[first_resolution].getFFTSize());
                for (auto& receiver : receivers_) {
                    receiver.prepare(2, static_cast<size_t>(history_size_));
                }
                for (size_t i = first_resolution; i < end_resolution; ++i) {
                    smoothers_[i].prepare(processors_[i].getFFTSize());
                }
                to_update_smooth = true;
                if (high_quality) {
                    frequencies_ = zldsp::analyzer::SpectrumBlender::createFrequencyGrid(
                        processors_[kLowResolution].getFFTSize(),
                        processors_[kMiddleResolution].getFFTSize(),
                        processors_[kHighResolution].getFFTSize(), sample_rate);
                } else {
                    const auto fft_size = processors_[kMiddleResolution].getFFTSize();
                    const auto delta_freq = static_cast<float>(sample_rate / static_cast<double>(fft_size));
                    frequencies_.resize(fft_size / 2 + 1);
                    for (size_t i = 0; i < frequencies_.size(); ++i) {
                        frequencies_[i] = static_cast<float>(i) * delta_freq;
                    }
                }
                spectrum_tilter_.prepareSpectrum(frequencies_.size());
                for (auto& decayer : spectrum_decayers_) {
                    decayer.prepareSpectrum(frequencies_.size());
                }
                xs_.resize(frequencies_.size());
                ys_.resize(xs_.size());
                if (high_quality) {
                    inter_.reset();
                } else {
                    inter_ = std::make_unique<zldsp::interpolation::SeqMakima<float>>(
                        xs_.data(), ys_.data(), kInterSize / 2 + 2, 0.f, 0.f);
                }
                for (size_t source = 0; source < kNumSources; ++source) {
                    spectra_[source].resize(frequencies_.size());
                    for (size_t resolution = first_resolution; resolution < end_resolution; ++resolution) {
                        resolution_spectra_[source][resolution].resize(
                            processors_[resolution].getFFTSize() / 2 + 1);
                    }
                }
                to_update_xs = true;
            }
            const auto fft_smooth_oct_value_idx = static_cast<int>(std::round(
                fft_smooth_oct_value_idx_ref_.load(std::memory_order::relaxed)));
            const auto fft_smooth_erb_value_idx = static_cast<int>(std::round(
                fft_smooth_erb_value_idx_ref_.load(std::memory_order::relaxed)));
            const auto fft_smooth_type_idx = static_cast<int>(std::round(
                fft_smooth_type_idx_ref_.load(std::memory_order::relaxed)));
            if (fft_smooth_oct_value_idx != fft_smooth_oct_value_idx_ ||
                fft_smooth_erb_value_idx != fft_smooth_erb_value_idx_ ||
                fft_smooth_type_idx != fft_smooth_type_idx_) {
                fft_smooth_oct_value_idx_ = fft_smooth_oct_value_idx;
                fft_smooth_erb_value_idx_ = fft_smooth_erb_value_idx;
                fft_smooth_type_idx_ = fft_smooth_type_idx;
                to_update_smooth = true;
            }
            if (to_update_smooth) {
                for (size_t i = first_resolution; i < end_resolution; ++i) {
                    if (fft_smooth_type_idx == 0) {
                        smoothers_[i].setSmoothOCT(
                            zlstate::PFFTSmoothOCTValue::kValues[static_cast<size_t>(fft_smooth_oct_value_idx)]);
                    } else {
                        smoothers_[i].setSmoothERB(sample_rate,
                            zlstate::PFFTSmoothERBValue::kValues[static_cast<size_t>(fft_smooth_erb_value_idx)]);
                    }
                }
            }
            auto& fifo{sender.getAbstractFIFO()};
            auto num_read = fifo.getNumReady();
            if (num_read > history_size_) {
                num_read = num_read / 2;
            }
            if (num_read > history_size_) {
                (void)fifo.prepareToRead(num_read - history_size_);
                fifo.finishRead(num_read - history_size_);
                num_read = history_size_;
            }
            const auto range = fifo.prepareToRead(num_read);
            for (size_t i = 0; i < receivers_.size(); ++i) {
                receivers_[i].pull(range, sender.getSampleFIFOs()[i]);
            }
            fifo.finishRead(num_read);
        }
        if (!std::isfinite(sample_rate) || sample_rate <= 20.0) {
            return;
        }
        if (history_size_ <= 0) {
            return;
        }
        const auto fft_tilt_idx = static_cast<int>(std::round(
            fft_tilt_idx_ref_.load(std::memory_order::relaxed)));
        if (fft_tilt_idx != fft_tilt_idx_) {
            fft_tilt_idx_ = fft_tilt_idx;
            to_update_tilt_.store(true, std::memory_order::relaxed);
        }
        if (to_update_tilt_.exchange(false, std::memory_order::acquire)) {
            const auto slope = zlstate::PFFTTilt::kSlopes[static_cast<size_t>(fft_tilt_idx_)] +
                fft_extra_tilt_.load(std::memory_order::relaxed);
            if (high_quality) {
                spectrum_tilter_.setTiltSlope(frequencies_, slope);
            } else {
                spectrum_tilter_.setTiltSlope(sample_rate, slope);
            }
        }
        if (to_update_xs || std::abs(bound.getWidth() - c_width_) > 0.1f) {
            c_width_ = bound.getWidth();
            const auto temp_scale = static_cast<float>(1.0 / std::log(sample_rate * 0.5 / 10.0)) * bound.getWidth();
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            for (size_t i = 1; i < xs_.size(); ++i) {
                xs_[i] = std::log(frequencies_[i]) * temp_scale - temp_bias;
            }
            xs_[0] = std::log(frequencies_[1] * .5f) * temp_scale - temp_bias;
            if (!high_quality) {
                inter_xs_[0] = xs_[0];
                inter_xs_[kInterSize] = xs_[kInterSize / 2];
                inter_xs_[kInterSize + 1] = xs_[kInterSize / 2 + 1];
                const auto delta_inter_x = (xs_[kInterSize / 2 - 1] - xs_[0]) / static_cast<float>(kInterSize - 1);
                for (size_t i = 1; i < kInterSize; ++i) {
                    inter_xs_[i] = inter_xs_[i - 1] + delta_inter_x;
                }
            }
        }
        const auto fft_speed_idx = static_cast<int>(std::round(
            fft_speed_idx_ref_.load(std::memory_order::relaxed)));
        if (fft_speed_idx != fft_speed_idx_) {
            fft_speed_idx_ = fft_speed_idx;
            to_update_decay_.store(true, std::memory_order::relaxed);
        }
        if (to_update_decay_.exchange(false, std::memory_order::acquire)) {
            const auto refresh_rate = refresh_rate_.load(std::memory_order::relaxed);
            const auto decay_rate = zlstate::PFFTSpeed::kFFTSpeed[
                static_cast<size_t>(fft_speed_idx_)] * fft_extra_speed_.load(std::memory_order::relaxed);
            spectrum_decayers_[0].setDecaySpeed(refresh_rate, c_fft_min_db_, static_cast<float>(0.15 / decay_rate));
            spectrum_decayers_[1].setDecaySpeed(refresh_rate, c_fft_min_db_, static_cast<float>(0.15 / decay_rate));
        }

        auto calculate_path = [&](const size_t source, juce::Path& path) {
            auto& spectrum = spectra_[source];
            if (high_quality) {
                for (size_t resolution = 0; resolution < kNumResolutions; ++resolution) {
                    auto& resolution_spectrum = resolution_spectra_[source][resolution];
                    receivers_[source].forward(processors_[resolution], zldsp::analyzer::StereoType::kStereo,
                                                resolution_spectrum);
                    zldsp::vector::multiply(resolution_spectrum.data(), noise_power_scales_[resolution],
                                            resolution_spectrum.size());
                    smoothers_[resolution].smooth(resolution_spectrum);
                }
                zldsp::analyzer::SpectrumBlender::blend(
                    spectrum, frequencies_,
                    resolution_spectra_[source][kLowResolution],
                    resolution_spectra_[source][kMiddleResolution],
                    resolution_spectra_[source][kHighResolution], sample_rate);
            } else {
                receivers_[source].forward(processors_[kMiddleResolution], zldsp::analyzer::StereoType::kStereo,
                                            spectrum);
                smoothers_[kMiddleResolution].smooth(spectrum);
            }
            zldsp::vector::sqr_mag_to_db(spectrum.data(), spectrum.size());
            spectrum_tilter_.tilt(spectrum);
            spectrum_decayers_[source].decay(spectrum, is_fft_frozen_.load(std::memory_order::relaxed));
            zldsp::vector::multiply(ys_.data(), spectrum.data(), bound.getHeight() / min_db, spectrum.size());

            path.clear();
            PathMinimizer<5> minimizer{path};
            size_t first_point{0};
            if (high_quality) {
                path.startNewSubPath(xs_[0], ys_[0]);
            } else {
                inter_->prepare();
                inter_->eval(inter_xs_.data(), inter_ys_.data(), inter_xs_.size());
                path.startNewSubPath(inter_xs_[0], inter_ys_[0]);
                for (size_t j = 1; j < kInterSize; j += 2) {
                    path.quadraticTo(inter_xs_[j], inter_ys_[j], inter_xs_[j + 1], inter_ys_[j + 1]);
                }
                path.lineTo(inter_xs_[kInterSize], inter_ys_[kInterSize]);
                first_point = kInterSize / 2;
            }
            minimizer.startNewSubPath<false>(xs_[first_point], ys_[first_point]);
            for (size_t j = first_point + 1; j < xs_.size(); ++j) {
                minimizer.lineTo(xs_[j], ys_[j]);
            }
            minimizer.finish();
        };

        calculate_path(0, next_out_path1_);
        calculate_path(1, next_out_path2_);

        std::lock_guard lock{mutex_};
        out_path1_.swapWithPath(next_out_path1_);
        out_path2_.swapWithPath(next_out_path2_);
    }

    void FFTAnalyzerPanel::setRefreshRate(const double refresh_rate) {
        refresh_rate_.store(static_cast<float>(refresh_rate), std::memory_order::relaxed);
        to_update_decay_.store(true, std::memory_order::release);
    }

    void FFTAnalyzerPanel::lookAndFeelChanged() {
        fft_extra_tilt_.store(base_.getFFTExtraTilt(), std::memory_order::relaxed);
        const auto extra_speed = base_.getFFTExtraSpeed();
        fft_extra_speed_.store(extra_speed * extra_speed + .1f, std::memory_order::relaxed);

        to_update_tilt_.store(true, std::memory_order::release);
        to_update_decay_.store(true, std::memory_order::release);
    }
}
