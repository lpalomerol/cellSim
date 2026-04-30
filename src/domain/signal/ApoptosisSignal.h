#pragma once

#include "BaseSignal.h"
#include <string>
#include <vector>

namespace domain {

    /**
     * ApoptosisSignal: emitted by the tissue's immune surveillance system.
     * Targeted at PRIMER-stage cells; the cell resolves fate in phase2_Endocytosis
     * based on its D2 immunosuppression accumulator.
     */
    class ApoptosisSignal : public BaseSignal {
    public:
        explicit ApoptosisSignal(std::uint64_t source_id,
                                 std::string message = "apoptosis",
                                 std::vector<std::uint64_t> targets = {})
            : BaseSignal(source_id, std::move(message), std::move(targets)) {}

        Type type() const noexcept override { return Type::Apoptosis; }
    };

} // namespace domain
