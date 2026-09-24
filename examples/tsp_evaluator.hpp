#ifndef TSP_EVALUATOR_HPP_
#define TSP_EVALUATOR_HPP_

#include <algorithm>
#include <cstdint>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>
#include <metah/operators/generator.hpp>
#include <metah/problem.hpp>
#include <metah/util/rng.hpp>


class Tsp final : public metah::Problem<std::vector<uint16_t>, int64_t, true> {
  size_t cities_;
  std::vector<ScoreType> distances_;

 public:
  Tsp(const size_t cities, std::vector<ScoreType> distances)
      : cities_(cities), distances_(std::move(distances)) {
    if (this->distances_.size() != cities * cities) {
      throw std::invalid_argument("Distances size must be equal to cities^2");
    }
  }

  // Assuming path starts and ends in the last city.
  [[nodiscard]] ScoreType evaluate(const ReprType& repr) const override {
    size_t lastCity = cities_ - 1;
    ScoreType totalDistance = distances_[repr.back() * cities_ + lastCity];
    for (const auto city : repr) {
      totalDistance += distances_[lastCity * cities_ + city];
      lastCity = city;
    }
    return totalDistance;
  }

  [[nodiscard]] size_t getCities() const { return cities_; }
};

template <typename RNG = metah::ThreadLocalRandom>
class TspRandomGenerator final : public metah::Generator<Tsp> {
  [[no_unique_address]] RNG rng_;

 public:
  explicit TspRandomGenerator(RNG rng = RNG()) : rng_(std::move(rng)) {}

  // Last city is implicit start/end of route, so it is not in representation
  [[nodiscard]] ReprType generateRepr(const Tsp& problem) override {
    ReprType repr(problem.getCities() - 1);
    std::iota(std::begin(repr), std::end(repr), 0);
    std::ranges::shuffle(repr, rng_.randomEngine());
    return repr;
  }

  [[nodiscard]] std::unique_ptr<Generator> clone() const override {
    return std::make_unique<TspRandomGenerator>(*this);
  }
};

#endif  // TSP_EVALUATOR_HPP_
