#ifndef SRC_METAH_ALGORITHMS_TABU_SEARCH_HPP_
#define SRC_METAH_ALGORITHMS_TABU_SEARCH_HPP_

#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "metah/observer.hpp"
#include "metah/operators/generator.hpp"
#include "metah/operators/neighbour_gen.hpp"
#include "metah/operators/tabu_list.hpp"
#include "metah/optimizer.hpp"

namespace metah {
/**
 * @brief OpenMP tabu search selecting the best admissible neighbor each iteration.
 * @tparam P Problem type; isMinimizing determines the optimization direction.
 * @tparam M Copyable, default-constructible move type.
 * @tparam T Owned tabu-list type providing contains(move) const and add(move).
 *
 * A tabu move is admissible when its candidate improves the best score seen.
 * If no admissible candidate exists, the current solution and tabu list are retained.
 * @note Neighbor generation, evaluation, and tabu lookup may run concurrently.
 * Tabu insertion has exclusive access. Exceptions must not escape parallel callbacks.
 * @note Register each non-null observer once, before run(), and keep it alive during it.
 * Observers receive the current solution, which may be worse than getBest().
 */
template <typename P, typename M, typename T = FifoTabuList<M>> class TabuSearch : public Optimizer<P> {
  public:
    using Base = Optimizer<P>;
    using typename Base::ProblemType;
    using typename Base::ReprType;
    using typename Base::ScoreType;
    using typename Base::SolutionType;
    using MoveType = M;
    using TabuListType = T;

  private:
    size_t iterations_;
    size_t threads_;
    size_t chunk_size_;
    std::unique_ptr<NeighbourGen<ReprType, MoveType>> neighbour_gen_;
    TabuListType tabu_list_;
    SolutionType current_solution_;
    SolutionType best_solution_;
    std::vector<SolutionObserver<ReprType, ScoreType> *> observers_;

    [[nodiscard]] static bool isBetter(const ScoreType &lhs, const ScoreType &rhs) {
        if constexpr (ProblemType::isMinimizing) {
            return lhs < rhs;
        } else {
            return lhs > rhs;
        }
    }

    void notifyObservers(size_t iteration = 0) {
        if (!observers_.empty()) {
#pragma omp for schedule(dynamic) nowait
            for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(observers_.size()); ++j) {
                observers_[j]->update(iteration, current_solution_);
            }
        }
    }

  public:
    /**
     * @brief Generates the initial solution and takes ownership of the neighbor generator and tabu list.
     * The generator is borrowed only during construction. An lvalue list is copied;
     * an rvalue is moved, preserving existing tabu entries in either case.
     * @param evaluator Shared problem used to evaluate candidates.
     * @param iterations Neighborhood search iterations per run(); zero only notifies observers.
     * @param tabu_list Configured tabu memory owned by the search.
     * @param threads Positive OpenMP thread count.
     * @param chunk_size Positive number of candidates per dynamic scheduling chunk.
     * @param neighbour_gen Neighbor generator whose ownership is transferred to the search.
     * @param generator Borrowed generator used to create the initial solution.
     * @pre threads and chunk_size are positive.
     * @throws std::invalid_argument if evaluator or neighbour_gen is null.
     */
    TabuSearch(std::shared_ptr<const ProblemType> evaluator, const uint64_t iterations, TabuListType tabu_list,
               const size_t threads, const size_t chunk_size,
               std::unique_ptr<NeighbourGen<ReprType, MoveType>> neighbour_gen, Generator<ProblemType> &generator)
        : Base(std::move(evaluator)), iterations_(iterations), threads_(threads), chunk_size_(chunk_size),
          neighbour_gen_(neighbour_gen ? std::move(neighbour_gen)
                                       : throw std::invalid_argument("Neighbour generator must not be null")),
          tabu_list_(std::move(tabu_list)), current_solution_(generator.generateSolution(this->getProblem())),
          best_solution_(current_solution_) {}

    /// @brief Returns a copy of the best solution seen; not safe concurrently with run().
    [[nodiscard]] SolutionType getBest() const override { return best_solution_; }

    /// @brief Registers an observer whose updates may overlap the next neighborhood evaluation.
    /// @param observer Non-null borrowed instance; register once and keep alive during run().
    void addObserver(SolutionObserver<ReprType, ScoreType> *observer) { observers_.push_back(observer); }

    /**
     * @brief Runs the configured number of additional iterations, retaining solutions and tabu history.
     * Observers are notified at iteration zero and after each iteration; numbering
     * restarts on each call. Do not run the same instance concurrently.
     */
    void run() override {
        SolutionType best_neighbour = current_solution_;
        MoveType best_move{};
        bool found_any_global{false};
#pragma omp parallel default(none) num_threads(threads_) shared(best_neighbour, best_move, found_any_global)
        {
            notifyObservers();
            SolutionType neighbour_solution = current_solution_;
            SolutionType local_best_neighbour = current_solution_;
            MoveType local_best_move{};

            for (size_t i = 1; i <= iterations_; ++i) {
                bool found_any_local = false;
#pragma omp for schedule(dynamic, chunk_size_) nowait
                for (std::ptrdiff_t j = 0;
                     j < static_cast<std::ptrdiff_t>(neighbour_gen_->size(current_solution_.repr)); ++j) {
                    const MoveType move = neighbour_gen_->generate(current_solution_.repr, neighbour_solution.repr, j);
                    neighbour_solution.score = this->getProblem().evaluate(neighbour_solution.repr);

                    const bool aspired = isBetter(neighbour_solution.score, best_solution_.score);
                    if (!aspired && tabu_list_.contains(move)) {
                        continue;
                    }

                    if (!found_any_local || isBetter(neighbour_solution.score, local_best_neighbour.score)) {
                        local_best_neighbour = neighbour_solution;
                        local_best_move = move;
                        found_any_local = true;
                    }
                }
                if (found_any_local) {
#pragma omp critical
                    {
                        if (!found_any_global || isBetter(local_best_neighbour.score, best_neighbour.score)) {
                            best_neighbour = local_best_neighbour;
                            best_move = local_best_move;
                            found_any_global = true;
                        }
                    }
                }
#pragma omp barrier
#pragma omp single
                {
                    if (found_any_global) {
                        current_solution_ = best_neighbour;
                        tabu_list_.add(best_move);
                        if (isBetter(current_solution_.score, best_solution_.score)) {
                            best_solution_ = current_solution_;
                        }
                    }
                    found_any_global = false;
                }
                notifyObservers(i);
            }
        }
    }
};

template <typename ProblemType, typename TabuListType, typename NeighbourGenType>
TabuSearch(std::shared_ptr<const ProblemType> evaluator, size_t iterations, TabuListType tabu_list, size_t threads,
           size_t chunk_size, std::unique_ptr<NeighbourGenType> neighbour_gen, Generator<ProblemType> &generator)
    -> TabuSearch<ProblemType, typename NeighbourGenType::MoveType, TabuListType>;

template <typename ProblemType, typename TabuListType, typename NeighbourGenType>
TabuSearch(std::shared_ptr<ProblemType> evaluator, size_t iterations, TabuListType tabu_list, size_t threads,
           size_t chunk_size, std::unique_ptr<NeighbourGenType> neighbour_gen, Generator<ProblemType> &generator)
    -> TabuSearch<ProblemType, typename NeighbourGenType::MoveType, TabuListType>;

/**
 * @brief Configures tabu search; requires a neighbor generator, solution generator, and tabu list.
 * Defaults to 100 iterations, one thread, and a scheduling chunk of one.
 */
template <typename P, typename M, typename T = FifoTabuList<M>> class TabuSearchBuilder {
  public:
    using ProblemType = P;
    using ReprType = ProblemType::ReprType;
    using ScoreType = ProblemType::ScoreType;
    using MoveType = M;
    using TabuListType = T;

  private:
    std::shared_ptr<const ProblemType> problem_;
    size_t iterations_ = 100;
    std::optional<TabuListType> tabu_list_;
    size_t threads_ = 1;
    size_t chunk_size_ = 1;
    std::unique_ptr<NeighbourGen<ReprType, MoveType>> neighbour_gen_;
    std::unique_ptr<Generator<ProblemType>> generator_;

    void validate() const {
        if (!neighbour_gen_) {
            throw std::invalid_argument("Neighbour generator must be set");
        }
        if (!generator_) {
            throw std::invalid_argument("Generator must be set");
        }
        if (!tabu_list_) {
            throw std::invalid_argument("Tabu list must be set");
        }
    }

  public:
    /// @brief Starts configuration with a shared problem; validation occurs in build().
    explicit TabuSearchBuilder(std::shared_ptr<const ProblemType> p) : problem_(std::move(p)) {}

    /// @brief Sets the shared problem used by subsequently built optimizers.
    TabuSearchBuilder &problem(std::shared_ptr<const ProblemType> p) {
        problem_ = std::move(p);
        return *this;
    }

    /// @brief Sets the number of iterations per run(); zero performs only initial notification.
    TabuSearchBuilder &iterations(const size_t iterations) {
        iterations_ = iterations;
        return *this;
    }

    /// @brief Stores a configured list by copy or move, including any existing tabu entries.
    TabuSearchBuilder &tabuList(TabuListType tabu_list) {
        tabu_list_.emplace(std::move(tabu_list));
        return *this;
    }

    /// @brief Sets the positive OpenMP thread count.
    TabuSearchBuilder &threads(const size_t threads) {
        threads_ = threads;
        return *this;
    }

    /// @brief Sets the positive chunk size for dynamically scheduled neighbor evaluation.
    TabuSearchBuilder &chunkSize(const size_t chunk_size) {
        chunk_size_ = chunk_size;
        return *this;
    }

    /// @brief Takes ownership of the generator used to initialize each built optimizer.
    TabuSearchBuilder &generator(std::unique_ptr<Generator<ProblemType>> generator) {
        generator_ = std::move(generator);
        return *this;
    }

    /// @brief Takes ownership of the neighbor generator configuration.
    TabuSearchBuilder &neighbour(std::unique_ptr<NeighbourGen<ReprType, MoveType>> neighbour_gen) {
        neighbour_gen_ = std::move(neighbour_gen);
        return *this;
    }

    /// @brief Builds using a copied tabu list and cloned neighbor generator.
    /// @throws std::invalid_argument if the problem or a required component is missing.
    [[nodiscard]] TabuSearch<ProblemType, MoveType, TabuListType> build() const & {
        validate();

        return TabuSearch<ProblemType, MoveType, TabuListType>(problem_, iterations_, *tabu_list_, threads_,
                                                               chunk_size_, neighbour_gen_->clone(), *generator_);
    }

    /// @brief Builds by moving the tabu list and neighbor generator out of this builder.
    /// @throws std::invalid_argument if the problem or a required component is missing.
    [[nodiscard]] TabuSearch<ProblemType, MoveType, TabuListType> build() && {
        validate();

        return TabuSearch<ProblemType, MoveType, TabuListType>(problem_, iterations_, std::move(*tabu_list_), threads_,
                                                               chunk_size_, std::move(neighbour_gen_), *generator_);
    }
};
} // namespace metah

#endif // SRC_METAH_ALGORITHMS_TABU_SEARCH_HPP_
