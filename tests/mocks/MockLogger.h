#pragma once

#include <string>
#include <gmock/gmock.h>
#include "../../src/domain/ports/ILogger.h"

namespace domain::ports {

class MockLogger : public ILogger {
public:
    MOCK_METHOD(void, logTissue, (const std::string&), (override));
    MOCK_METHOD(void, logCell, (const std::string&), (override));
    MOCK_METHOD(void, logGenome, (const std::string&), (override));
};

} // namespace domain::ports

