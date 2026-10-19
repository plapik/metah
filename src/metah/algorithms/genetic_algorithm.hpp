#ifndef SRC_METAH_ALGORITHMS_GENETIC_ALGORITHM_HPP_
#define SRC_METAH_ALGORITHMS_GENETIC_ALGORITHM_HPP_

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "metah/observer.hpp"
#include "metah/operators/crossover.hpp"
#include "metah/operators/generator.hpp"
#include "metah/operators/mutate.hpp"
#include "metah/operators/selection.hpp"
#include "metah/optimizer.hpp"
#include "metah/problem.hpp"

namespace metah {
/**
 * @brief Generational genetic algorithm with OpenMP offspring generation and evaluation.
 * @tparam P Problem type; isMinimizing determines the optimization direction.
 *
 * Retains the best solution seen, without automatically preserving it in the population.
 * @note Evaluation, selection, crossover, and mutation may run concurrently on
 * shared instances. Exceptions must not escape parallel callbacks.
 * @note Register each non-null observer once, before run(), and keep it alive during it.
 * Observers receive the current population and scores.
 */
template <typename P> class GeneticAlgorithm final : public Optimizer<P> {
  public:
    using Base = Optimizer<P>;

    using typename Base::ProblemType;
    using typename Base::ReprType;
    using typename Base::ScoreType;
    using typename Base::SolutionType;

    using Population = std::vector<ReprType>;
    using Scores = std::vector<ScoreType>;
    using PopObserver = PopulationObserver<ReprType, ScoreType>;

  private:
    size_t population_size_;
    uint64_t generations_;
    size_t num_threads_;
    size_t chunk_size_;
    std::unique_ptr<Selection<ScoreType>> select_;
    std::unique_ptr<Crossover<ReprType>> cross_;
    std::unique_ptr<Mutate<ReprType>> mutate_;
    Population population_;
    Scores scores_;
    SolutionType best_solution_;
    std::vector<PopObserver *> low_prio_observers_;
    std::vector<PopObserver *> high_prio_observers_;

    void update_best_solution() {
        size_t best_idx = 0;
        auto best_score = scores_[0];
        for (size_t j = 1; j < population_size_; ++j) {
            if constexpr (ProblemType::isMinimizing) {
                if (scores_[j] < best_score) {
                    best_score = scores_[j];
                    best_idx = j;
                }
            } else {
                if (scores_[j] > best_score) {
                    best_score = scores_[j];
                    best_idx = j;
                }
            }
        }
        if constexpr (ProblemType::isMinimizing) {
            if (best_score < best_solution_.score) {
                best_solution_.repr = population_[best_idx];
                best_solution_.score = best_score;
            }
        } else {
            if (best_score > best_solution_.score) {
                best_solution_.repr = population_[best_idx];
                best_solution_.score = best_score;
            }
        }
    }

    void notifyObservers(uint64_t iteration = 0) {
        if (!high_prio_observers_.empty()) {
#pragma omp for schedule(dynamic)
            for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(high_prio_observers_.size()); ++j) {
                high_prio_observers_[j]->update(iteration, population_, scores_);
            }
        }
        if (!low_prio_observers_.empty()) {
#pragma omp for schedule(dynamic) nowait
            for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(low_prio_observers_.size()); ++j) {
                low_prio_observers_[j]->update(iteration, population_, scores_);
            }
        }
    }

  public:
    /**
     * @brief Generates the initial population and takes ownership of selection, crossover, and mutation.
     * Odd population sizes are rounded down to an even number. The generator is
     * borrowed only during construction.
     * @param problem Shared problem used to evaluate population members.
     * @param population_size Population count, at least two; odd values are rounded down.
     * @param generations Number of generations per run(); zero only notifies observers.
     * @param threads Positive OpenMP thread count.
     * @param chunk_size Positive number of offspring pairs per dynamic scheduling chunk.
     * @param select Selection operator whose ownership is transferred to the algorithm.
     * @param cross Crossover operator whose ownership is transferred to the algorithm.
     * @param mutate Mutation operator whose ownership is transferred to the algorithm.
     * @param generator Borrowed generator used to create the initial population.
     * @throws std::invalid_argument if population_size < 2, threads or chunk_size
     * is zero, or the problem or an operator is null.
     */
    GeneticAlgorithm(std::shared_ptr<const ProblemType> problem, const size_t population_size, const size_t generations,
                     const size_t threads, const size_t chunk_size, std::unique_ptr<Selection<ScoreType>> select,
                     std::unique_ptr<Crossover<ReprType>> cross, std::unique_ptr<Mutate<ReprType>> mutate,
                     Generator<ProblemType> &generator)
        : Base(std::move(problem)),
          population_size_(population_size >= 2 ? population_size & ~size_t{1}
                                                : throw std::invalid_argument("Population size must be at least 2")),
          generations_(generations),
          num_threads_(threads > 0 ? threads : throw std::invalid_argument("Number of threads must be positive")),
          chunk_size_(chunk_size > 0 ? chunk_size : throw std::invalid_argument("Chunk size must be positive")),
          select_(select ? std::move(select) : throw std::invalid_argument("Selection method must not be null")),
          cross_(cross ? std::move(cross) : throw std::invalid_argument("Crossover method must not be null")),
          mutate_(mutate ? std::move(mutate) : throw std::invalid_argument("Mutation method must not be null")),
          best_solution_(generator.generateSolution(this->getProblem())) {
        population_.reserve(this->population_size_);
        scores_.reserve(this->population_size_);
        population_.emplace_back(best_solution_.repr);
        scores_.emplace_back(best_solution_.score);
        for (size_t i = 1; i < this->population_size_; ++i) {
            population_.emplace_back(generator.generateRepr(this->getProblem()));
            scores_.emplace_back(this->getProblem().evaluate(population_[i]));
        }
        update_best_solution();
    }

    /// @brief Returns a copy of the best solution seen; not safe concurrently with run().
    [[nodiscard]] SolutionType getBest() const override { return best_solution_; }

    /**
     * @brief Runs the configured number of additional generations, retaining the population and best solution.
     * Observers are notified at generation zero and after each generation; numbering
     * restarts on each call. Do not run the same instance concurrently.
     */
    void run() override {
        auto next_pop = population_;
        auto next_scores = scores_;

#pragma omp parallel num_threads(num_threads_) default(none) shared(next_pop, next_scores)
        {
            notifyObservers();
            for (uint64_t i = 1; i <= generations_; ++i) {
#pragma omp for schedule(dynamic, chunk_size_)
                for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(population_size_); j += 2) {
                    const auto idx1 = select_->select(scores_);
                    const auto idx2 = select_->select(scores_);
                    auto &child1 = next_pop[j];
                    auto &child2 = next_pop[j + 1];
                    auto modified1 = cross_->cross(population_[idx1], population_[idx2], child1, child2);
                    auto modified2 = modified1;
                    modified1 = mutate_->mutate(child1) || modified1;
                    modified2 = mutate_->mutate(child2) || modified2;
                    next_scores[j] = modified1 ? this->getProblem().evaluate(child1) : scores_[idx1];
                    next_scores[j + 1] = modified2 ? this->getProblem().evaluate(child2) : scores_[idx2];
                }
#pragma omp single
                {
                    std::swap(population_, next_pop);
                    std::swap(scores_, next_scores);
                }
                notifyObservers(i);
#pragma omp single nowait
                update_best_solution();
            }
        }
    }

    /// @brief Registers an observer whose updates may overlap the next offspring generation.
    /// @param observer Non-null borrowed instance; register once and keep alive during run().
    void addLowPrioObserver(PopObserver *observer) { low_prio_observers_.push_back(observer); }

    /**
     * @brief Registers a blocking observer: execution waits for all high-priority updates before proceeding.
     * Callbacks may run on different workers, but the next offspring generation
     * cannot begin until they all finish.
     * @param observer Non-null borrowed instance; register once and keep alive during run().
     */
    void addHighPrioObserver(PopObserver *observer) { high_prio_observers_.push_back(observer); }

    ~GeneticAlgorithm() override = default;
};

template <typename ProblemType>
GeneticAlgorithm(std::shared_ptr<const ProblemType> problem, size_t, size_t, size_t, size_t,
                 std::unique_ptr<Selection<typename ProblemType::ScoreType>>,
                 std::unique_ptr<Crossover<typename ProblemType::ReprType>>,
                 std::unique_ptr<Mutate<typename ProblemType::ReprType>>, Generator<ProblemType> &)
    -> GeneticAlgorithm<ProblemType>;

template <typename ProblemType>
GeneticAlgorithm(std::shared_ptr<ProblemType> problem, size_t, size_t, size_t, size_t,
                 std::unique_ptr<Selection<typename ProblemType::ScoreType>>,
                 std::unique_ptr<Crossover<typename ProblemType::ReprType>>,
                 std::unique_ptr<Mutate<typename ProblemType::ReprType>>, Generator<ProblemType> &)
    -> GeneticAlgorithm<ProblemType>;

/**
 * @brief Configures a genetic algorithm; requires selection, crossover, mutation, and a generator.
 * Defaults to a population of 100, 100 generations, one thread, and a scheduling chunk of one.
 */
template <typename P> class GeneticAlgorithmBuilder {
  public:
    using ProblemType = P;
    using ReprType = ProblemType::ReprType;
    using ScoreType = ProblemType::ScoreType;

  private:
    std::shared_ptr<const ProblemType> problem_;
    size_t population_size_ = 100;
    size_t generations_ = 100;
    size_t threads_ = 1;
    size_t chunk_size_ = 1;
    std::unique_ptr<Selection<ScoreType>> select_;
    std::unique_ptr<Crossover<ReprType>> cross_;
    std::unique_ptr<Mutate<ReprType>> mutate_;
    std::unique_ptr<Generator<ProblemType>> generator_;

  public:
    /// @brief Starts configuration with a shared problem; validation occurs in build().
    explicit GeneticAlgorithmBuilder(std::shared_ptr<const ProblemType> p) : problem_(std::move(p)) {}

    /// @brief Sets the shared problem used by subsequently built optimizers.
    GeneticAlgorithmBuilder &problem(std::shared_ptr<const ProblemType> p) {
        problem_ = std::move(p);
        return *this;
    }

    /// @brief Sets the population size, at least two; odd values are rounded down during construction.
    GeneticAlgorithmBuilder &populationSize(const size_t population_size) {
        population_size_ = population_size;
        return *this;
    }

    /// @brief Sets the generations per run(); zero performs only initial notification.
    GeneticAlgorithmBuilder &generations(const size_t generations) {
        generations_ = generations;
        return *this;
    }

    /// @brief Sets the positive OpenMP thread count.
    GeneticAlgorithmBuilder &threads(const size_t threads) {
        threads_ = threads;
        return *this;
    }

    /// @brief Sets the positive chunk size, measured in pairs of offspring.
    GeneticAlgorithmBuilder &chunkSize(const size_t chunk_size) {
        chunk_size_ = chunk_size;
        return *this;
    }

    /// @brief Takes ownership of the selection configuration.
    GeneticAlgorithmBuilder &selection(std::unique_ptr<Selection<ScoreType>> selection) {
        select_ = std::move(selection);
        return *this;
    }

    /// @brief Takes ownership of the crossover configuration.
    GeneticAlgorithmBuilder &crossover(std::unique_ptr<Crossover<ReprType>> crossover) {
        cross_ = std::move(crossover);
        return *this;
    }

    /// @brief Takes ownership of the mutation configuration.
    GeneticAlgorithmBuilder &mutation(std::unique_ptr<Mutate<ReprType>> mutation) {
        mutate_ = std::move(mutation);
        return *this;
    }

    /// @brief Takes ownership of the generator used to initialize each built optimizer.
    GeneticAlgorithmBuilder &generator(std::unique_ptr<Generator<ProblemType>> generator) {
        generator_ = std::move(generator);
        return *this;
    }

    /// @brief Builds an initialized optimizer from cloned operators and the configured generator.
    /// @throws std::invalid_argument if configuration is invalid or a required component is missing.
    [[nodiscard]] GeneticAlgorithm<ProblemType> build() const {
        if (!select_) {
            throw std::invalid_argument("Selection method must be set");
        }
        if (!cross_) {
            throw std::invalid_argument("Crossover method must be set");
        }
        if (!mutate_) {
            throw std::invalid_argument("Mutation method must be set");
        }
        if (!generator_) {
            throw std::invalid_argument("Generator must be set");
        }

        return GeneticAlgorithm<ProblemType>(problem_, population_size_, generations_, threads_, chunk_size_,
                                             select_->clone(), cross_->clone(), mutate_->clone(), *generator_);
    }
};

template <typename P> GeneticAlgorithmBuilder(std::shared_ptr<const P> problem) -> GeneticAlgorithmBuilder<P>;

template <typename P> GeneticAlgorithmBuilder(std::shared_ptr<P> problem) -> GeneticAlgorithmBuilder<P>;
} // namespace metah

#endif // SRC_METAH_ALGORITHMS_GENETIC_ALGORITHM_HPP_
