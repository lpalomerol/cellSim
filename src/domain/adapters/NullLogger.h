#pragma once

#include "../ports/ILogger.h"
#include <string>

namespace domain::adapters {

class NullLogger : public ports::ILogger {
public:
    void logTissue(const std::string& message) override {
        // No hace nada
    }

    void logCell(const std::string& message) override {
        // No hace nada
    }

    void logGenome(const std::string& message) override {
        // No hace nada
    }
};

} // namespace domain::adapters

