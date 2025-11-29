#pragma once

#include <iostream>
#include <string>
#include "../ports/ILogger.h"
#include "LogLevel.h"

namespace domain::adapters {

class Logger : public ports::ILogger {
private:
    LogLevel config_;

public:
    explicit Logger(const LogLevel& config = LogLevel())
        : config_(config) {}

    void setLogLevel(const LogLevel& config) {
        config_ = config;
    }

    const LogLevel& getLogLevel() const {
        return config_;
    }

    void logTissue(const std::string& message) override {
        if (config_.tissue) {
            std::cout << "[TISSUE] " << message << "\n";
        }
    }

    void logCell(const std::string& message) override {
        if (config_.cell) {
            std::cout << "[CELL] " << message << "\n";
        }
    }

    void logGenome(const std::string& message) override {
        if (config_.genome) {
            std::cout << "[GENOME] " << message << "\n";
        }
    }
};

} // namespace domain::adapters

