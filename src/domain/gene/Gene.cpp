//
// Created by luis on 31/10/25.
//

#include "Gene.h"

#include <iostream>
#include <utility>

namespace domain {

    Gene::Gene(std::string name, State initial, double mutation_threshold, double mutation_instability_k, bool verbose)
        : name_(std::move(name)), state_(initial), mutation_threshold_(mutation_threshold), mutation_instability_k_(mutation_instability_k), noise_(nullptr), verbose_(verbose) {}

    const std::string& Gene::name() const {
        return name_;
    }

    void Gene::mutate() {
        switch (state_) {
            case State::PlusPlus:
                state_ = State::PlusMinus;
                break;
            case State::PlusMinus:
                state_ = State::MinusMinus;
                break;
            case State::MinusMinus:
                break;
        }
    }

    bool Gene::enabled() const {
        return state_ != State::MinusMinus;
    }

    std::string Gene::status() const {
        switch (state_) {
            case State::PlusPlus: return "+/+";
            case State::PlusMinus: return "+/-";
            case State::MinusMinus: return "-/-";
            default: return "?";
        }
    }

    std::string Gene::details() const {
        // Mostrar el threshold efectivo considerando inestabilidad (clamped internamente)
        return name_ + " [" + status() + "] p(mut)=" + std::to_string(get_mutation_threshold(true));
    }

    std::string Gene::details(bool unstable) const {
        return name_ + " [" + status() + "] p(mut)=" + std::to_string(get_mutation_threshold(unstable));
    }


    void Gene::live() {
        // Consumir una muestra desde la fuente de ruido y aplicar mutación si corresponde.
        live(true);
    }

    void Gene::live(bool apply_instability) {
        double threshold = get_mutation_threshold(apply_instability);
        if (noise_) {
            double sample = noise_->next().u01;
            if (sample < threshold) {
                if (verbose_) std::cout << "[Gene::live] Gene " << name_ << " mutating (sample=" << sample << " > threshold=" << threshold << ")\n";
                mutate();
            } else {
                if (verbose_) std::cout << "[Gene::live] Gene " << name_ << " not mutating (sample=" << sample << " <= threshold=" << threshold << ")\n";
            }
        }
    }

    void Gene::live(bool apply_instability, double immunosuppression) {
        // immunosuppression acts as a multiplicative degrader: values >1 increase effective mutation probability
        domain::shared::Threshold t = mutation_threshold_; // copy
        if (apply_instability) t += mutation_instability_k_;
        t *= immunosuppression; // Threshold clamps internally
        double threshold = t.value();

        if (noise_) {
            double sample = noise_->next().u01;
            if (sample < threshold) {
                if (verbose_) std::cout << "[Gene::live] Gene " << name_ << " mutating (sample=" << sample << " < threshold=" << threshold << ")\n";
                mutate();
            } else {
                if (verbose_) std::cout << "[Gene::live] Gene " << name_ << " not mutating (sample=" << sample << " >= threshold=" << threshold << ")\n";
            }
        }
    }

    void Gene::setNoiseSource(INoiseSource* noise) {
        noise_ = noise;
    }

    void Gene::setState(State s) {
        state_ = s;
    }

    void Gene::setVerbose(bool v) { verbose_ = v; }

    double Gene::get_mutation_threshold(bool apply_instability) const {
        // Use the Threshold value object to apply instability and keep limits enforced by Threshold itself.
        domain::shared::Threshold t = mutation_threshold_; // copy
        if (apply_instability) t += mutation_instability_k_;
        return t.value();
    }
}