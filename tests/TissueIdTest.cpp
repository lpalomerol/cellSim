// filepath: /home/luis/CLionProjects/cellSim/tests/TissueIdTest.cpp
#include <gtest/gtest.h>
#include <unordered_set>
#include "../src/domain/tissue/Tissue.h"
#include "../src/domain/ports/ICell.h"

using namespace domain;

// Minimal test cell implementing id storage
struct TestCell : public ICell {
    std::uint64_t cell_id_ = static_cast<std::uint64_t>(-1);
    int calls = 0;
    void live() override { ++calls; }
    bool alive() const override { return true; }
    bool isNeoplastic() const override { return false; }
    void details() const override {}
    void mutateGene(const std::string& name) override {}
    void setId(std::uint64_t id) override { cell_id_ = id; }
    std::uint64_t id() const override { return cell_id_; }
};

TEST(TissueIdTest, AssignsIncrementalIds) {
    Tissue t;
    auto c1 = std::make_unique<TestCell>();
    auto c2 = std::make_unique<TestCell>();

    t.addCell(std::move(c1));
    t.addCell(std::move(c2));

    ASSERT_EQ(t.size(), 2u);

    const ICell* a = t.getCell(0);
    const ICell* b = t.getCell(1);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);

    EXPECT_EQ(a->id(), 0u);
    EXPECT_EQ(b->id(), 1u);
}

TEST(TissueIdTest, IdsAreUnique) {
    Tissue t;
    const std::size_t N = 50;
    for (std::size_t i = 0; i < N; ++i) {
        t.addCell(std::make_unique<TestCell>());
    }
    ASSERT_EQ(t.size(), N);

    std::unordered_set<std::uint64_t> ids;
    for (std::size_t i = 0; i < N; ++i) {
        const ICell* c = t.getCell(i);
        ASSERT_NE(c, nullptr);
        ids.insert(c->id());
    }
    EXPECT_EQ(ids.size(), N);
}

TEST(TissueIdTest, TissueHasOwnId) {
    Tissue t;
    EXPECT_EQ(t.id(), static_cast<std::uint64_t>(-1));
    t.setId(42);
    EXPECT_EQ(t.id(), 42u);
}


