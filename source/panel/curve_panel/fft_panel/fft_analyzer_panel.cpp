// Copyright (C) 2026 - zsliu98
// This file is part of ZLSplitter
//
// ZLSplitter is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License Version 3 as published by the Free Software Foundation.
//
// ZLSplitter is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License along with ZLSplitter. If not, see <https://www.gnu.org/licenses/>.

#include "fft_analyzer_panel.hpp"

namespace zlpanel {
    FFTAnalyzerPanel::FFTAnalyzerPanel(PluginProcessor& p, zlgui::UIBase& base) :
        p_ref_(p),
        base_(base),
        split_type_ref_(*p.parameters_.getRawParameterValue(zlp::PSplitType::kID)),
        swap_ref_(*p.parameters_.getRawParameterValue(zlp::PSwap::kID)),
        fft_min_db_ref_(*p.na_parameters_.getRawParameterValue(zlstate::PFFTMinDB::kID)) {
        constexpr auto preallocateSpace = static_cast<int>(zlp::Controller<double>::kAnalyzerPointNum) * 3 + 1;
        for (auto& path : {&out_path1_, &out_path2_, &next_out_path1_, &next_out_path2_}) {
            path->preallocateSpace(preallocateSpace);
        }
        receiver_.setON({true, true});
        setInterceptsMouseClicks(false, false);
    }

    FFTAnalyzerPanel::~FFTAnalyzerPanel() = default;

    void FFTAnalyzerPanel::paint(juce::Graphics& g) {
        const std::unique_lock lock{mutex_, std::try_to_lock};
        if (!lock.owns_lock()) {
            return;
        }
        const auto thickness = base_.getFontSize() * .2f * base_.getFFTCurveThickness();
        if (static_cast<zlp::PSplitType::SplitType>(
            std::round(split_type_ref_.load(std::memory_order_relaxed))) == zlp::PSplitType::kNone) {
            g.setColour(base_.getTextColor());
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
        const auto min_db = zlstate::PFFTMinDB::kDBs[static_cast<size_t>(std::round(
            fft_min_db_ref_.load(std::memory_order::relaxed)))];
        bool to_update_xs_{false};
        double sample_rate;
        {
            auto& sender{p_ref_.getController().getAnalyzerSender()};
            std::lock_guard lock{sender.getLock()};
            sample_rate = sender.getSampleRate();
            if (std::abs(c_sample_rate_ - sample_rate) > 0.1) {
                c_sample_rate_ = sample_rate;
                to_update_tilt_.store(true, std::memory_order::relaxed);

                int fft_order;
                if (sample_rate <= 50000) {
                    fft_order = 12;
                } else if (sample_rate <= 100000) {
                    fft_order = 13;
                } else if (sample_rate <= 200000) {
                    fft_order = 14;
                } else {
                    fft_order = 15;
                }
                fft_size_ = 1 << fft_order;
                receiver_.prepare(static_cast<int>(fft_order), {2, 2});
                spectrum_smoother_.prepare(static_cast<size_t>(fft_size_));
                spectrum_smoother_.setSmooth(1.0 / 12.0);
                spectrum_tilter_.prepare(static_cast<size_t>(fft_size_));
                spectrum_decayers_[0].prepare(static_cast<size_t>(fft_size_));
                spectrum_decayers_[1].prepare(static_cast<size_t>(fft_size_));

                xs_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
                y1s_.resize(static_cast<size_t>(fft_size_) / 2 + 1);
                y2s_.resize(static_cast<size_t>(fft_size_) / 2 + 1);

                to_update_xs_ = true;
            }

            auto& fifo{sender.getAbstractFIFO()};
            auto num_read = fifo.getNumReady();
            if (num_read > static_cast<int>(receiver_.getFFTSize())) {
                (void)fifo.prepareToRead(num_read - static_cast<int>(receiver_.getFFTSize()));
                fifo.finishRead(num_read - static_cast<int>(receiver_.getFFTSize()));
                num_read = static_cast<int>(receiver_.getFFTSize());
            }
            const auto range = fifo.prepareToRead(num_read);
            receiver_.pull(range, sender.getSampleFIFOs());
            fifo.finishRead(num_read);
        }
        if (sample_rate < 40000.0) {
            return;
        }
        if (fft_size_ <= 0) {
            return;
        }
        receiver_.forward(zldsp::analyzer::StereoType::kStereo);

        if (to_update_tilt_.exchange(false, std::memory_order::acquire)) {
            spectrum_tilter_.setTiltSlope(sample_rate,
                                          spectrum_tilt_slope_.load(std::memory_order::relaxed));
        }
        if (to_update_xs_ || std::abs(bound.getWidth() - c_width_) > 0.1f) {
            c_width_ = bound.getWidth();
            const auto delta_freq = static_cast<float>(sample_rate / static_cast<double>(fft_size_));
            const auto temp_scale = static_cast<float>(1.0 / std::log(sample_rate * 0.5 / 10.0)) * bound.getWidth();
            const auto temp_bias = std::log(static_cast<float>(10.0)) * temp_scale;
            for (size_t i = 1; i < xs_.size(); ++i) {
                const auto freq = delta_freq * static_cast<float>(i);
                xs_[i] = std::log(freq) * temp_scale - temp_bias;
            }
            xs_[0] = std::min(0.f, xs_[2] - 2.f * xs_[1]);
        }
        if (to_update_decay_.exchange(false, std::memory_order::acquire)) {
            spectrum_decayers_[0].setDecaySpeed(refresh_rate_.load(std::memory_order::relaxed),
                                            spectrum_decay_speed_.load(std::memory_order::relaxed));
            spectrum_decayers_[1].setDecaySpeed(refresh_rate_.load(std::memory_order::relaxed),
                                            spectrum_decay_speed_.load(std::memory_order::relaxed));
        }

        auto calculate_path = [&](kfr::univector<float>& spectrum,
            kfr::univector<float>& ys,
            zldsp::analyzer::SpectrumDecayer& decayer,
            juce::Path& path) {
            spectrum_smoother_.smooth(spectrum);
            spectrum = 10.f * kfr::log10(kfr::max(spectrum, 1e-24f));
            decayer.decay(spectrum, is_fft_frozen_.load(std::memory_order::relaxed));
            spectrum_tilter_.tilt(spectrum);
            ys = spectrum * (bound.getHeight() / min_db);
            path.clear();
            PathMinimizer<10> minimizer{path};
            minimizer.startNewSubPath<true>(xs_[0], ys[0]);
            for (size_t i = 1; i < xs_.size(); ++i) {
                minimizer.lineTo(xs_[i], ys[i]);
            }
            minimizer.finish();
        };

        calculate_path(receiver_.getAbsSqrFFTBuffers()[0], y1s_, spectrum_decayers_[0], next_out_path1_);
        calculate_path(receiver_.getAbsSqrFFTBuffers()[1], y2s_, spectrum_decayers_[1], next_out_path2_);

        std::lock_guard lock{mutex_};
        out_path1_.swapWithPath(next_out_path1_);
        out_path2_.swapWithPath(next_out_path2_);
    }

    void FFTAnalyzerPanel::setRefreshRate(const double refresh_rate) {
        refresh_rate_.store(static_cast<float>(refresh_rate), std::memory_order::relaxed);
        to_update_decay_.store(true, std::memory_order::release);
    }
}
