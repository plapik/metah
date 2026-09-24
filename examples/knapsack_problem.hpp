#ifndef KNAPSACK_PROBLEM_HPP_
#define KNAPSACK_PROBLEM_HPP_

#include <cassert>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>
#include <metah/operators/generator.hpp>
#include <metah/problem.hpp>
#include <metah/util/rng.hpp>

class KnapsackProblem final
    : public metah::Problem<std::vector<uint8_t>, int64_t, false> {
  std::vector<int64_t> weights;
  std::vector<int64_t> values;
  int64_t capacity;
  int64_t value_upper_bound;

 public:
  KnapsackProblem(std::vector<int64_t> weights, std::vector<int64_t> values,
                  const int64_t capacity)
      : weights(weights.size() == values.size() && !weights.empty()
                    ? std::move(weights)
                    : throw std::invalid_argument(
                          "Weights and values must have the same size")),
        values(std::move(values)),
        capacity(capacity),
        value_upper_bound(std::accumulate(this->values.begin(),
                                          this->values.end(), int64_t{0})) {}

  [[nodiscard]] int64_t evaluate(
      const std::vector<uint8_t>& repr) const override {
    assert(repr.size() == this->weights.size());

    int64_t value{0}, weight{0};
    for (size_t i = 0; i < weights.size(); ++i) {
      weight += weights[i] * repr[i];
    }
    for (size_t i = 0; i < values.size(); ++i) {
      value += values[i] * repr[i];
    }

    if (weight > capacity) {
      return value - value_upper_bound + capacity - weight;
    }
    return value;
  }

  [[nodiscard]] size_t getNumItems() const { return weights.size(); }
};

template <typename RNG = metah::ThreadLocalRandom>
class KnapsackRandomGenerator final : public metah::Generator<KnapsackProblem> {
  [[no_unique_address]] RNG rng_;
  double one_ratio_;

 public:
  explicit KnapsackRandomGenerator(const double one_ratio = 0.5,
                                   RNG rng = RNG())
      : rng_(std::move(rng)), one_ratio_(one_ratio) {}

  [[nodiscard]] ReprType generateRepr(const KnapsackProblem& problem) override {
    ReprType repr(problem.getNumItems());
    for (auto& item : repr) {
      item = static_cast<uint8_t>(rng_.randomBool(one_ratio_));
    }
    return repr;
  }

  [[nodiscard]] std::unique_ptr<Generator> clone() const override {
    return std::make_unique<KnapsackRandomGenerator>(*this);
  }
};

#endif  // KNAPSACK_PROBLEM_HPP_
