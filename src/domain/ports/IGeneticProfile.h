#pragma once

#include <string>

namespace domain {

    /**
     * IGeneticProfile: Interface for cells to expose their genetic information.
     * Allows Tissue to query genetic data without coupling to specific cell implementations.
     */
    class IGeneticProfile {
    public:
        virtual ~IGeneticProfile() = default;

        /**
         * Get BRCA1 gene status (e.g., "+/-", "-/-", "+/+")
         */
        [[nodiscard]] virtual std::string getBRCA1Status() const = 0;

        /**
         * Get TP53 gene status (e.g., "+/-", "-/-", "+/+")
         */
        [[nodiscard]] virtual std::string getTP53Status() const = 0;

        /**
         * Query if cell is neoplastic
         */
        [[nodiscard]] virtual bool isNeoplastic() const = 0;
    };

} // namespace domain

