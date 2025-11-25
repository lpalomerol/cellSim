// filepath: /home/luis/CLionProjects/cellSim/tests/NeoplasmSignalTest.cpp
#include <gtest/gtest.h>
#include "../src/domain/signal/ISignal.h"
#include "../src/domain/signal/NeoplasmSignal.h"

using namespace domain;

TEST(NeoplasmSignalTest, TypeAndSource) {
    NeoplasmSignal s(123, "cell mutated");
    EXPECT_EQ(s.type(), ISignal::Type::Neoplasm);
    EXPECT_EQ(s.sourceId(), 123u);
    EXPECT_STREQ(s.message().c_str(), "cell mutated");
}

