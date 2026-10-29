#pragma once

#include <string>
#include <memory>

namespace domain::ports {

class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void logTissue(const std::string& message) = 0;
    virtual void logCell(const std::string& message) = 0;
    virtual void logGenome(const std::string& message) = 0;
    virtual void setVerbose(bool verbose) = 0;
};

using ILoggerPtr = std::shared_ptr<ILogger>;

} // namespace domain::ports

