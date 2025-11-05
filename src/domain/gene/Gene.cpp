//
// Created by luis on 31/10/25.
//

#include "Gene.h"
#include <utility>

namespace domain {

    Gene::Gene(std::string name, State initial, double mutation_threshold, double mutation_instability_k)
        : name_(std::move(name)), state_(initial), mutation_threshold_(mutation_threshold), mutation_instability_k_(mutation_instability_k), noise_(nullptr) {}

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
        return name_ + " [" + status() + "] p(mut)=" + std::to_string(mutation_threshold_ + mutation_instability_k_);
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
            if (sample > threshold) {
                mutate();
            }
        }
    }

    void Gene::setNoiseSource(INoiseSource* noise) {
        noise_ = noise;
    }

    void Gene::setState(State s) {
        state_ = s;
    }

    double Gene::get_mutation_threshold(bool apply_instability) const {
        return mutation_threshold_  + (apply_instability ? mutation_instability_k_ : 0.0);
    }
}