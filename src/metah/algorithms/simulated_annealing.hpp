#ifndef SRC_METAH_ALGORITHMS_SIMULATED_ANNEALING_HPP_
#define SRC_METAH_ALGORITHMS_SIMULATED_ANNEALING_HPP_

#include <memory>
#include <stdexcept>
#include <vector>

#include "metah/observer.hpp"
#include "metah/operators/generator.hpp"
#include "metah/operators/perturbation.hpp"
#include "metah/optimizer.hpp"
#include "metah/util/rng.hpp"

namespace metah {
template <typename S>
class AcceptanceStrategy {
 public:
  using ScoreType = S;
  virtual bool shouldAccept(const ScoreType& current_score,
                            const ScoreType& neighbour_score,
                            double temperature) const = 0;
  [[nodiscard]] virtual std::unique_ptr<AcceptanceStrategy> clone() const = 0;
  virtual ~AcceptanceStrategy() = default;
};

template <typename S, bool isMinimizing, typename RNG = ThreadLocalRandom>
class MetropolisAcceptance final : public AcceptanceStrategy<S> {
  [[no_unique_address]] RNG rng_;

 public:
  using ScoreType = S;
  explicit MetropolisAcceptance(RNG rng = RNG()) : rng_(std::move(rng)) {}

  bool shouldAccept(const ScoreType& current_score,
                    const ScoreType& neighbour_score,
                    const double temperature) const override {
    if (temperature <= 0.0) {
      return false;
    }

    double delta;
    if constexpr (isMinimizing) {
      delta = static_cast<double>(current_score) -
              static_cast<double>(neighbour_score);
    } else {
      delta = static_cast<double>(neighbour_score) -
              static_cast<double>(current_score);
    }

    return std::exp(delta / temperature) > rng_.random01();
  }

  [[nodiscard]] std::unique_ptr<AcceptanceStrategy<S>> clone() const override {
    return std::make_unique<MetropolisAcceptance>(*this);
  }
};

class CoolingSchema {
 public:
  [[nodiscard]] virtual double newTemperature(double current_temperature,
                                              size_t iteration) const = 0;
  [[nodiscard]] virtual std::unique_ptr<CoolingSchema> clone() const = 0;
  virtual ~CoolingSchema() = default;
};

class GeometricCooling final : public CoolingSchema {
  double alpha_;

 public:
  explicit GeometricCooling(const double alpha) : alpha_(alpha) {
    if (alpha <= 0.0 || alpha >= 1.0) {
      throw std::invalid_argument(
          "Alpha must be strictly between 0.0 and 1.0 for Geometric Cooling.");
    }
  }

  [[nodiscard]] double newTemperature(const double current_temperature,
                                      size_t iteration) const override {
    return current_temperature * alpha_;
  }

  [[nodiscard]] std::unique_ptr<CoolingSchema> clone() const override {
    return std::make_unique<GeometricCooling>(*this);
  }
};

template <typename P>
class SimulatedAnnealing final : public Optimizer<P> {
 public:
  using Base = Optimizer<P>;
  using typename Base::ProblemType;
  using typename Base::ReprType;
  using typename Base::ScoreType;
  using typename Base::SolutionType;

 private:
  uint64_t iterations_;
  double temperature_;
  SolutionType best_solution_;
  SolutionType current_solution_;
  std::unique_ptr<Perturbation<ReprType>> perturbation_;
  std::unique_ptr<AcceptanceStrategy<ScoreType>> acceptance_;
  std::unique_ptr<CoolingSchema> cooling_schema_;
  std::vector<SolutionObserver<ReprType, ScoreType>*> high_prio_observers_;
  std::vector<SolutionObserver<ReprType, ScoreType>*> low_prio_observers_;

  void notifyObservers(size_t iteration = 0) {
    for (size_t j = 0; j < high_prio_observers_.size(); ++j) {
      high_prio_observers_[j]->update(iteration, current_solution_);
    }
    for (size_t j = 0; j < low_prio_observers_.size(); ++j) {
      low_prio_observers_[j]->update(iteration, current_solution_);
    }
  }

 public:
  SimulatedAnnealing(
      std::shared_ptr<const ProblemType> problem, const uint64_t iterations,
      const double temperature,
      std::unique_ptr<Perturbation<ReprType>> perturbation,
      std::unique_ptr<CoolingSchema> cooling_schema,
      std::unique_ptr<AcceptanceStrategy<ScoreType>> acceptance_prob,
      Generator<ProblemType>& generator)
      : Base(std::move(problem)),
        iterations_(iterations),
        temperature_(temperature),
        best_solution_(generator.generateSolution(this->getProblem())),
        current_solution_(this->best_solution_),
        perturbation_(perturbation ? std::move(perturbation)
                                  : throw std::invalid_argument(
                                        "perturbation must not be null")),
        acceptance_(acceptance_prob ? std::move(acceptance_prob)
                                   : throw std::invalid_argument(
                                         "acceptance_prob must not be null")),
        cooling_schema_(cooling_schema
                           ? std::move(cooling_schema)
                           : throw std::invalid_argument(
                                 "cooling_schema must not be null")) {}

  [[nodiscard]] SolutionType getBest() const override { return best_solution_; }

  void addHighPrioObserver(SolutionObserver<ReprType, ScoreType>* observer) {
    high_prio_observers_.push_back(observer);
  }

  void addLowPrioObserver(SolutionObserver<ReprType, ScoreType>* observer) {
    low_prio_observers_.push_back(observer);
  }

  void run() override {
    notifyObservers();
    SolutionType neighbour_solution = current_solution_;
    for (size_t i = 1; i <= iterations_; ++i) {
      perturbation_->perturb(current_solution_.repr, neighbour_solution.repr);
      neighbour_solution.score =
          this->getProblem().evaluate(neighbour_solution.repr);
      if constexpr (ProblemType::isMinimizing) {
        if (neighbour_solution.score < current_solution_.score) {
          current_solution_ = neighbour_solution;
          if (current_solution_.score < best_solution_.score) {
            best_solution_ = current_solution_;
          }
        } else if (acceptance_->shouldAccept(current_solution_.score,
                                            neighbour_solution.score,
                                            temperature_)) {
          current_solution_ = neighbour_solution;
        }
      } else {
        if (neighbour_solution.score > current_solution_.score) {
          current_solution_ = neighbour_solution;
          if (current_solution_.score > best_solution_.score) {
            best_solution_ = current_solution_;
          }
        } else if (acceptance_->shouldAccept(current_solution_.score,
                                            neighbour_solution.score,
                                            temperature_)) {
          current_solution_ = neighbour_solution;
        }
      }
      temperature_ = cooling_schema_->newTemperature(temperature_, i);
      notifyObservers(i);
    }
  }
};

template <typename ProblemType>
SimulatedAnnealing(
    std::shared_ptr<const ProblemType> problem, uint64_t iterations,
    double temperature,
    std::unique_ptr<Perturbation<typename ProblemType::ReprType>> perturbation,
    std::unique_ptr<CoolingSchema> cooling_schema,
    std::unique_ptr<AcceptanceStrategy<typename ProblemType::ScoreType>>
        acceptance_prob,
    Generator<ProblemType>& generator) -> SimulatedAnnealing<ProblemType>;

template <typename ProblemType>
SimulatedAnnealing(
    std::shared_ptr<ProblemType> problem, uint64_t iterations,
    double temperature,
    std::unique_ptr<Perturbation<typename ProblemType::ReprType>> perturbation,
    std::unique_ptr<CoolingSchema> cooling_schema,
    std::unique_ptr<AcceptanceStrategy<typename ProblemType::ScoreType>>
        acceptance_prob,
    Generator<ProblemType>& generator) -> SimulatedAnnealing<ProblemType>;

template <typename P>
class SimulatedAnnealingBuilder {
 public:
  using ProblemType = P;
  using ReprType = ProblemType::ReprType;
  using ScoreType = ProblemType::ScoreType;

 private:
  std::shared_ptr<const ProblemType> problem_;
  uint64_t iterations_ = 1000;
  double temperature_ = 100.0;
  std::unique_ptr<Perturbation<ReprType>> perturbation_;
  std::unique_ptr<CoolingSchema> cooling_schema_;
  std::unique_ptr<AcceptanceStrategy<ScoreType>> acceptance_;
  std::unique_ptr<Generator<ProblemType>> generator_;

 public:
  explicit SimulatedAnnealingBuilder(std::shared_ptr<const ProblemType> p)
      : problem_(std::move(p)) {}

  SimulatedAnnealingBuilder& problem(std::shared_ptr<const ProblemType> p) {
    problem_ = std::move(p);
    return *this;
  }

  SimulatedAnnealingBuilder& iterations(const uint64_t iterations) {
    iterations_ = iterations;
    return *this;
  }

  SimulatedAnnealingBuilder& temperature(const double temperature) {
    temperature_ = temperature;
    return *this;
  }

  SimulatedAnnealingBuilder& perturbation(
      std::unique_ptr<Perturbation<ReprType>> perturbation) {
    perturbation_ = std::move(perturbation);
    return *this;
  }

  SimulatedAnnealingBuilder& cooling(
      std::unique_ptr<CoolingSchema> cooling_schema) {
    cooling_schema_ = std::move(cooling_schema);
    return *this;
  }

  SimulatedAnnealingBuilder& acceptance(
      std::unique_ptr<AcceptanceStrategy<ScoreType>> acceptance) {
    acceptance_ = std::move(acceptance);
    return *this;
  }

  SimulatedAnnealingBuilder& generator(
      std::unique_ptr<Generator<ProblemType>> generator) {
    generator_ = std::move(generator);
    return *this;
  }

  [[nodiscard]] SimulatedAnnealing<ProblemType> build() const {
    if (!perturbation_) throw std::invalid_argument("Perturbation must be set");
    if (!cooling_schema_)
      throw std::invalid_argument("Cooling schema must be set");
    if (!acceptance_)
      throw std::invalid_argument("Acceptance strategy must be set");
    if (!generator_) throw std::invalid_argument("Generator must be set");

    return SimulatedAnnealing<ProblemType>(
        problem_, iterations_, temperature_, perturbation_->clone(),
        cooling_schema_->clone(), acceptance_->clone(), *generator_);
  }
};

template <typename P>
SimulatedAnnealingBuilder(std::shared_ptr<const P> problem)
    -> SimulatedAnnealingBuilder<P>;

template <typename P>
SimulatedAnnealingBuilder(std::shared_ptr<P> problem)
    -> SimulatedAnnealingBuilder<P>;
}  // namespace metah

#endif  // SRC_METAH_ALGORITHMS_SIMULATED_ANNEALING_HPP_
