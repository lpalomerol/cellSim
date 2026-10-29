#pragma once

namespace domain::adapters {

struct LogLevel {
    bool tissue = false;
    bool cell = false;
    bool genome = false;

    LogLevel() = default;

    LogLevel(bool t, bool c, bool g)
        : tissue(t), cell(c), genome(g) {}

    void enableAll() {
        tissue = cell = genome = true;
    }

    void disableAll() {
        tissue = cell = genome = false;
    }
};

} // namespace domain::adapters

