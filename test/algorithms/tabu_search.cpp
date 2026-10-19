#include <memory>
#include <vector>

#include <doctest/doctest.h>

#include <metah/algorithms/tabu_search.hpp>
#include <metah/problem.hpp>

namespace {
class TestProblem final : public metah::Problem<int, int, true> {
  public:
    int evaluate(const int &repr) const override { return repr; }
};

class TestGenerator final : public metah::Generator<TestProblem> {
  public:
    int generateRepr(const TestProblem &) override { return 0; }

    std::unique_ptr<Generator> clone() const override { return std::make_unique<TestGenerator>(*this); }
};

class TestNeighbour final : public metah::NeighbourGen<int, int> {
  public:
    size_t size(const int &) const override { return 2; }

    int generate(const int &original, int &target, const size_t idx) override {
        if (original == 0) {
            target = idx == 0 ? 1 : 2;
            return idx == 0 ? 1 : 2;
        }
        if (original == 1) {
            target = idx == 0 ? 0 : 3;
            return idx == 0 ? 1 : 3;
        }
        target = idx == 0 ? -1 : 4;
        return idx == 0 ? 1 : 4;
    }

    std::unique_ptr<NeighbourGen> clone() const override { return std::make_unique<TestNeighbour>(*this); }
};

class PathObserver final : public metah::SolutionObserver<int, int> {
  public:
    std::vector<int> path;

    void update(size_t, const int &repr, const int &) override { path.push_back(repr); }
};

} // namespace

TEST_SUITE_BEGIN("Tabu search");

TEST_CASE("Default FIFO blocks recent moves and allows aspiration") {
    auto problem = std::make_shared<TestProblem>();
    TestGenerator generator;
    PathObserver observer;
    metah::TabuSearch search(problem, 3, metah::FifoTabuList<int>(2), 2, 1, std::make_unique<TestNeighbour>(),
                             generator);
    search.addHighPrioObserver(&observer);
    search.run();

    CHECK_EQ(observer.path, std::vector<int>{0, 1, 3, -1});
    CHECK_EQ(search.getBest().score, -1);
}

TEST_CASE("Tabu search owns a copy of the supplied list") {
    auto problem = std::make_shared<TestProblem>();
    TestGenerator generator;
    PathObserver observer;
    metah::FifoTabuList<int> list(1);
    list.add(1);

    metah::TabuSearch search(problem, 1, list, 1, 1, std::make_unique<TestNeighbour>(), generator);
    list.add(2);
    search.addHighPrioObserver(&observer);
    search.run();

    CHECK_EQ(observer.path, std::vector<int>{0, 2});
    CHECK(list.contains(2));
    CHECK_FALSE(list.contains(1));
}

TEST_CASE("Tabu search builder accepts a configured list") {
    auto problem = std::make_shared<TestProblem>();
    metah::FifoTabuList<int> list(2);
    list.add(1);
    metah::TabuSearchBuilder<TestProblem, int> builder(problem);
    builder.iterations(1)
        .tabuList(std::move(list))
        .generator(std::make_unique<TestGenerator>())
        .neighbour(std::make_unique<TestNeighbour>());

    auto copied_search = builder.build();
    auto moved_search = std::move(builder).build();
    PathObserver copied_observer;
    PathObserver moved_observer;
    copied_search.addHighPrioObserver(&copied_observer);
    moved_search.addHighPrioObserver(&moved_observer);
    copied_search.run();
    moved_search.run();

    CHECK_EQ(copied_observer.path, std::vector<int>{0, 2});
    CHECK_EQ(moved_observer.path, std::vector<int>{0, 2});
}

TEST_CASE("Tabu search builder requires a tabu list") {
    metah::TabuSearchBuilder<TestProblem, int> builder(std::make_shared<TestProblem>());
    builder.generator(std::make_unique<TestGenerator>()).neighbour(std::make_unique<TestNeighbour>());

    CHECK_THROWS_WITH_AS(static_cast<void>(builder.build()), "Tabu list must be set", std::invalid_argument);
}

TEST_SUITE_END();
