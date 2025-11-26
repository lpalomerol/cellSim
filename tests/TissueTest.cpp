// filepath: /home/luis/CLionProjects/cellSim/tests/TissueTest.cpp
#include <gtest/gtest.h>
#include "../src/domain/ports/ICell.h"
#include "../src/domain/tissue/Tissue.h"

using namespace domain;

// Minimal fake cell to count live() calls
struct FakeCell : public ICell {
    int calls = 0;
    void live() override { ++calls; }
    bool alive() const override { return true; }
    bool isNeoplastic() const override { return false; }
    void mutateGene(const std::string& name) override {}

    // IGeneticProfile implementation
    [[nodiscard]] std::string getBRCA1Status() const override { return "+/-"; }
    [[nodiscard]] std::string getTP53Status() const override { return "+/+"; }
};

// Fake that throws in live()
struct ThrowingCell : public ICell {
    void live() override { throw std::runtime_error("boom"); }
    bool alive() const override { return true; }
    bool isNeoplastic() const override { return false; }
    void mutateGene(const std::string& name) override {}

    // IGeneticProfile implementation
    [[nodiscard]] std::string getBRCA1Status() const override { return "+/-"; }
    [[nodiscard]] std::string getTP53Status() const override { return "+/+"; }
};

TEST(TissueTest, AddAndSize) {
    Tissue t;
    EXPECT_EQ(t.size(), 0u);
    t.addCell(std::make_unique<FakeCell>());
    EXPECT_EQ(t.size(), 1u);
}

TEST(TissueTest, LiveInvokesCells) {
    Tissue t;
    auto c1 = std::make_unique<FakeCell>();
    auto c2 = std::make_unique<FakeCell>();
    FakeCell* p1 = c1.get();
    FakeCell* p2 = c2.get();
    t.addCell(std::move(c1));
    t.addCell(std::move(c2));

    t.live();

    EXPECT_EQ(p1->calls, 1);
    EXPECT_EQ(p2->calls, 1);
}

TEST(TissueTest, ExceptionInCellDoesNotStopOthers) {
    Tissue t;
    auto good = std::make_unique<FakeCell>();
    auto bad = std::make_unique<ThrowingCell>();
    FakeCell* pg = good.get();
    t.addCell(std::move(bad));
    t.addCell(std::move(good));

    // Should not throw
    EXPECT_NO_THROW(t.live());
    EXPECT_EQ(pg->calls, 1);
}


