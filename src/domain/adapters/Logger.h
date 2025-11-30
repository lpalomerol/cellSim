#pragma once

#include <iostream>
#include <string>
#include "../ports/ILogger.h"
#include "../ports/ILoggeable.h"
#include "LogLevel.h"

namespace domain::adapters {

class Logger : public ports::ILogger {
private:
    LogLevel config_;
    bool verbose_ = false;

public:
    explicit Logger(const LogLevel& config = LogLevel())
        : config_(config) {}

    void setLogLevel(const LogLevel& config) {
        config_ = config;
    }

    const LogLevel& getLogLevel() const {
        return config_;
    }

    void setVerbose(bool verbose) override {
        verbose_ = verbose;
        if (verbose) {
            config_.enableAll();
        } else {
            config_.disableAll();
        }
    }

    void logTissue(const std::string& message) override {
        if (config_.tissue && verbose_) {
            std::cout << "[TISSUE] " << message << "\n";
        }
    }

    void logCell(const std::string& message) override {
        if (config_.cell && verbose_) {
            std::cout << "[CELL] " << message << "\n";
        }
    }

    void logGenome(const std::string& message) override {
        if (config_.genome && verbose_) {
            std::cout << "[GENOME] " << message << "\n";
        }
    }

    // Nuevo: método genérico que usa ILoggeable
    void log(const ports::ILoggeable* obj, const std::string& message) {
        if (!obj || !verbose_) return;

        std::string category = obj->getLogCategory();
        bool should_log = isVerboseCategory(category);

        if (should_log) {
            std::cout << "[" << category << "] " << message << "\n";
        }
    }

private:
    bool isVerboseCategory(const std::string& category) const {
        if (category == "TISSUE") return config_.tissue;
        if (category == "CELL") return config_.cell;
        if (category == "GENOME") return config_.genome;
        return false;
    }
};

} // namespace domain::adapters

