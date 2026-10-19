#ifndef SRC_METAH_ALGORITHMS_SIMULATED_ANNEALING_HPP_
#define SRC_METAH_ALGORITHMS_SIMULATED_ANNEALING_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "metah/observer.hpp"
#include "metah/operators/acceptance_strategy.hpp"
#include "metah/operators/cooling_schema.hpp"
#include "metah/operators/generator.hpp"
#include "metah/operators/perturbation.hpp"
#include "metah/optimizer.hpp"

namespace metah {
/**
 * @brief Serial simulated annealing with pluggable perturbation, acceptance, and cooling.
 * @tparam P Problem type; isMinimizing determines the optimization direction.
 *
 * Strict improvements are always accepted; other candidates use the acceptance
 * strategy. Cooling is applied after every iteration, including rejected moves.
 * @note Register each non-null observer once, before run(), and keep it alive during it.
 * Observers receive the current solution, which may be worse than getBest().
 */
template <typename P> class SimulatedAnnealing final : public Optimizer<P> {
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
    std::vector<SolutionObserver<ReprType, ScoreType> *> high_prio_observers_;
    std::vector<SolutionObserver<ReprType, ScoreType> *> low_prio_observers_;

    void notifyObservers(size_t iteration = 0) {
        for (size_t j = 0; j < high_prio_observers_.size(); ++j) {
            high_prio_observers_[j]->update(iteration, current_solution_);
        }
        for (size_t j = 0; j < low_prio_observers_.size(); ++j) {
            low_prio_observers_[j]->update(iteration, current_solution_);
        }
    }

  public:
    /**
     * @brief Generates the initial solution, stores the initial temperature, and takes ownership of the operators.
     * The generator is borrowed only during construction. Temperature requirements
     * are defined by the selected acceptance and cooling operators.
     * @param problem Shared problem used to evaluate candidates.
     * @param iterations Candidate evaluations per run(); zero only notifies observers.
     * @param temperature Initial temperature, valid for the selected acceptance and cooling operators.
     * @param perturbation Operator producing candidates; ownership is transferred to the algorithm.
     * @param cooling_schema Temperature schedule; ownership is transferred to the algorithm.
     * @param acceptance_prob Acceptance strategy; ownership is transferred to the algorithm.
     * @param generator Borrowed generator used to create the initial solution.
     * @throws std::invalid_argument if the problem or any operator is null.
     */
    SimulatedAnnealing(std::shared_ptr<const ProblemType> problem, const uint64_t iterations, const double temperature,
                       std::unique_ptr<Perturbation<ReprType>> perturbation,
                       std::unique_ptr<CoolingSchema> cooling_schema,
                       std::unique_ptr<AcceptanceStrategy<ScoreType>> acceptance_prob,
                       Generator<ProblemType> &generator)
        : Base(std::move(problem)), iterations_(iterations), temperature_(temperature),
          best_solution_(generator.generateSolution(this->getProblem())), current_solution_(this->best_solution_),
          perturbation_(perturbation ? std::move(perturbation)
                                     : throw std::invalid_argument("perturbation must not be null")),
          acceptance_(acceptance_prob ? std::move(acceptance_prob)
                                      : throw std::invalid_argument("acceptance_prob must not be null")),
          cooling_schema_(cooling_schema ? std::move(cooling_schema)
                                         : throw std::invalid_argument("cooling_schema must not be null")) {}

    /// @brief Returns a copy of the best solution seen; not safe concurrently with run().
    [[nodiscard]] SolutionType getBest() const override { return best_solution_; }

    /// @brief Registers a blocking observer called before low-priority observers; run() waits for it to return.
    /// @param observer Non-null borrowed instance; register once and keep alive during run().
    void addHighPrioObserver(SolutionObserver<ReprType, ScoreType> *observer) {
        high_prio_observers_.push_back(observer);
    }

    /// @brief Registers an observer called after high-priority observers; it also blocks this serial algorithm.
    /// @param observer Non-null borrowed instance; register once and keep alive during run().
    void addLowPrioObserver(SolutionObserver<ReprType, ScoreType> *observer) {
        low_prio_observers_.push_back(observer);
    }

    /**
     * @brief Runs additional iterations from the retained solution and temperature, without reheating.
     * Observer and cooling iteration numbers restart on each call. Observers are
     * notified at iteration zero and after each iteration. Do not run the same instance concurrently.
     */
    void run() override {
        notifyObservers();
        SolutionType neighbour_solution = current_solution_;
        for (size_t i = 1; i <= iterations_; ++i) {
            perturbation_->perturb(current_solution_.repr, neighbour_solution.repr);
            neighbour_solution.score = this->getProblem().evaluate(neighbour_solution.repr);
            if constexpr (ProblemType::isMinimizing) {
                if (neighbour_solution.score < current_solution_.score) {
                    current_solution_ = neighbour_solution;
                    if (current_solution_.score < best_solution_.score) {
                        best_solution_ = current_solution_;
                    }
                } else if (acceptance_->shouldAccept(current_solution_.score, neighbour_solution.score, temperature_)) {
                    current_solution_ = neighbour_solution;
                }
            } else {
                if (neighbour_solution.score > current_solution_.score) {
                    current_solution_ = neighbour_solution;
                    if (current_solution_.score > best_solution_.score) {
                        best_solution_ = current_solution_;
                    }
                } else if (acceptance_->shouldAccept(current_solution_.score, neighbour_solution.score, temperature_)) {
                    current_solution_ = neighbour_solution;
                }
            }
            temperature_ = cooling_schema_->newTemperature(temperature_, i);
            notifyObservers(i);
        }
    }
};

template <typename ProblemType>
SimulatedAnnealing(std::shared_ptr<const ProblemType> problem, uint64_t iterations, double temperature,
                   std::unique_ptr<Perturbation<typename ProblemType::ReprType>> perturbation,
                   std::unique_ptr<CoolingSchema> cooling_schema,
                   std::unique_ptr<AcceptanceStrategy<typename ProblemType::ScoreType>> acceptance_prob,
                   Generator<ProblemType> &generator) -> SimulatedAnnealing<ProblemType>;

template <typename ProblemType>
SimulatedAnnealing(std::shared_ptr<ProblemType> problem, uint64_t iterations, double temperature,
                   std::unique_ptr<Perturbation<typename ProblemType::ReprType>> perturbation,
                   std::unique_ptr<CoolingSchema> cooling_schema,
                   std::unique_ptr<AcceptanceStrategy<typename ProblemType::ScoreType>> acceptance_prob,
                   Generator<ProblemType> &generator) -> SimulatedAnnealing<ProblemType>;

/**
 * @brief Configures simulated annealing; requires perturbation, acceptance, cooling, and a generator.
 * Defaults to 1000 iterations and an initial temperature of 100.
 */
template <typename P> class SimulatedAnnealingBuilder {
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
    /// @brief Starts configuration with a shared problem; validation occurs in build().
    explicit SimulatedAnnealingBuilder(std::shared_ptr<const ProblemType> p) : problem_(std::move(p)) {}

    /// @brief Sets the shared problem used by subsequently built optimizers.
    SimulatedAnnealingBuilder &problem(std::shared_ptr<const ProblemType> p) {
        problem_ = std::move(p);
        return *this;
    }

    /// @brief Sets the number of iterations per run(); zero performs only initial notification.
    SimulatedAnnealingBuilder &iterations(const uint64_t iterations) {
        iterations_ = iterations;
        return *this;
    }

    /// @brief Sets the initial temperature; use a value valid for the selected operators.
    SimulatedAnnealingBuilder &temperature(const double temperature) {
        temperature_ = temperature;
        return *this;
    }

    /// @brief Takes ownership of the perturbation configuration.
    SimulatedAnnealingBuilder &perturbation(std::unique_ptr<Perturbation<ReprType>> perturbation) {
        perturbation_ = std::move(perturbation);
        return *this;
    }

    /// @brief Takes ownership of the cooling schedule configuration.
    SimulatedAnnealingBuilder &cooling(std::unique_ptr<CoolingSchema> cooling_schema) {
        cooling_schema_ = std::move(cooling_schema);
        return *this;
    }

    /// @brief Takes ownership of the acceptance strategy configuration.
    SimulatedAnnealingBuilder &acceptance(std::unique_ptr<AcceptanceStrategy<ScoreType>> acceptance) {
        acceptance_ = std::move(acceptance);
        return *this;
    }

    /// @brief Takes ownership of the generator used to initialize each built optimizer.
    SimulatedAnnealingBuilder &generator(std::unique_ptr<Generator<ProblemType>> generator) {
        generator_ = std::move(generator);
        return *this;
    }

    /// @brief Builds an initialized optimizer from cloned operators and the configured generator.
    /// @throws std::invalid_argument if the problem or a required component is missing.
    [[nodiscard]] SimulatedAnnealing<ProblemType> build() const {
        if (!perturbation_) {
            throw std::invalid_argument("Perturbation must be set");
        }
        if (!cooling_schema_) {
            throw std::invalid_argument("Cooling schema must be set");
        }
        if (!acceptance_) {
            throw std::invalid_argument("Acceptance strategy must be set");
        }
        if (!generator_) {
            throw std::invalid_argument("Generator must be set");
        }

        return SimulatedAnnealing<ProblemType>(problem_, iterations_, temperature_, perturbation_->clone(),
                                               cooling_schema_->clone(), acceptance_->clone(), *generator_);
    }
};

template <typename P> SimulatedAnnealingBuilder(std::shared_ptr<const P> problem) -> SimulatedAnnealingBuilder<P>;

template <typename P> SimulatedAnnealingBuilder(std::shared_ptr<P> problem) -> SimulatedAnnealingBuilder<P>;
} // namespace metah

#endif // SRC_METAH_ALGORITHMS_SIMULATED_ANNEALING_HPP_
