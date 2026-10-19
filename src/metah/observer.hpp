#ifndef SRC_METAH_OBSERVER_HPP_
#define SRC_METAH_OBSERVER_HPP_

#include <span>

#include "metah/solution.hpp"

namespace metah {
/**
 * @brief Receives solution updates during optimization.
 * @note Algorithms assume observers may not be thread-safe. Different observer
 * instances may be updated concurrently; shared state requires synchronization.
 * Registered instances must outlive every run that uses them.
 */
template <typename R, typename S> class SolutionObserver {
  public:
    using ReprType = R;
    using ScoreType = S;
    /**
     * @brief Observes a solution. Copy any data that must outlive the callback.
     * @param iteration Completed steps in the current run; zero reports its starting state.
     * @param repr Current representation, not necessarily the best found so far.
     * @param score Objective value corresponding to repr.
     */
    virtual void update(size_t iteration, const ReprType &repr, const ScoreType &score) = 0;

    /// @brief Forwards a solution and its score to the observer implementation.
    /// @param iteration Completed steps in the current run; zero reports its starting state.
    /// @param solution Current representation paired with its score.
    void update(const size_t iteration, const Solution<ReprType, ScoreType> &solution) {
        update(iteration, solution.repr, solution.score);
    }

    virtual ~SolutionObserver() = default;
};

/**
 * @brief Receives population updates during optimization.
 * @note Algorithms assume observers may not be thread-safe. Different observer
 * instances may be updated concurrently; shared state requires synchronization.
 * Registered instances must outlive every run that uses them.
 */
template <typename R, typename S> class PopulationObserver {
  public:
    using ReprType = R;
    using ScoreType = S;
    /**
     * @brief Observes a population. Copy any data that must outlive the callback.
     * @param iteration Completed generations in the current run; zero reports its starting state.
     * @param population Current population representations.
     * @param scores Objective values corresponding to population entries in the same order.
     */
    virtual void update(size_t iteration, std::span<const ReprType> population, std::span<const ScoreType> scores) = 0;
    virtual ~PopulationObserver() = default;
};
} // namespace metah

#endif // SRC_METAH_OBSERVER_HPP_
