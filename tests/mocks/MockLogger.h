#pragma once

#include <gmock/gmock.h>
#include "src/domain/ports/ILogger.h"

class MockLogger : public domain::ports::ILogger {
public:
    MOCK_METHOD(void, logTissue, (const std::string&), (override));
    MOCK_METHOD(void, logCell, (const std::string&), (override));
    MOCK_METHOD(void, logGenome, (const std::string&), (override));
};

