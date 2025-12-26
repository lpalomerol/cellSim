
#include "Gene.h"
#include "GeneConstants.h"
#include <cassert>
#include "../adapters/NullLogger.h"
#include <utility>

namespace domain {

    Gene::Gene(std::string name,
        State initial,
        double mutation_threshold,
        double mutation_instability_k,
        ports::ILoggerPtr logger)
        : name_(std::move(name)), state_(initial),
    mutation_threshold_(mutation_threshold),
    mutation_instability_k_(mutation_instability_k),
    noise_(nullptr),
    logger_(logger ? logger : std::make_shared<adapters::NullLogger>()) {
        assert(!name_.empty());
        assert(mutation_threshold >= 0.0 && mutation_threshold <= 1.0);
        assert(mutation_instability_k >= 0.0);
    }

    const std::string& Gene::name() const {
        return name_;
    }

    GeneStatus Gene::getStatus() const {
        switch (state_) {
            case State::PlusPlus:
                return GeneStatus::enabled();
            case State::PlusMinus:
                return GeneStatus::partiallyEnabled();
            case State::MinusMinus:
                return GeneStatus::disabled();
            default:
                return GeneStatus::unknown();
        }
    }

    void Gene::mutate() {
        if (state_ == State::PlusPlus) {
            state_ = State::PlusMinus;
        } else if (state_ == State::PlusMinus) {
            state_ = State::MinusMinus;
        }
    }

    bool Gene::enabled() const {
        return state_ != State::MinusMinus;
    }

    std::string Gene::status() const {
        static const char* statuses[] = {
            GeneticStatusStrings::WILD_TYPE,
            GeneticStatusStrings::HETEROZYGOUS,
            GeneticStatusStrings::HOMOZYGOUS_RECESSIVE
        };
        return statuses[static_cast<int>(state_)];
    }

    std::string Gene::details(bool apply_instability) const {
        return name_ + " [" + status() + "] p(mut)=" + std::to_string(calculateMutationThreshold(apply_instability, 1.0));
    }

    void Gene::live(bool apply_instability, double genomic_instability) {
        assert(genomic_instability > 0.0);
        assert(noise_ != nullptr);

        double threshold = calculateMutationThreshold(apply_instability, genomic_instability);
        double sample = noise_->next().u01;
        bool should_mutate = sample < threshold;

        logger_->logGenome("[Gene::live] Gene " + name_ + (should_mutate ? " mutating" : " not mutating") +
            " (sample=" + std::to_string(sample) + " " + (should_mutate ? "<" : ">=") + " threshold=" + std::to_string(threshold) + ")");

        if (should_mutate) {
            mutate();
        }
    }

    double Gene::calculateMutationThreshold(bool apply_instability, double genomic_instability) const {
        domain::shared::Threshold t = mutation_threshold_;
        if (apply_instability) {
            t += mutation_instability_k_;
        }
        t *= genomic_instability;
        return t.value();
    }

    void Gene::setNoiseSource(INoiseSource* noise) {
        assert(noise != nullptr);
        noise_ = noise;
    }

    void Gene::setState(State s) {
        state_ = s;
    }

}
