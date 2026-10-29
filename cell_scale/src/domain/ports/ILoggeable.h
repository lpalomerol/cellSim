#pragma once

#include <string>

namespace domain::ports {

class ILoggeable {
public:
    virtual ~ILoggeable() = default;
    virtual std::string getLogCategory() const = 0;
};

}

