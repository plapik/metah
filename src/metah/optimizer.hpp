#ifndef SRC_METAH_OPTIMIZER_HPP_
#define SRC_METAH_OPTIMIZER_HPP_

#include <memory>
#include <utility>

namespace metah {
/**
 * @brief Common interface for algorithms that optimize a shared problem.
 * @tparam P Problem type defining the representation, score, and solution types.
 */
template <typename P> class Optimizer {
  public:
    using ProblemType = P;
    using ReprType = ProblemType::ReprType;
    using ScoreType = ProblemType::ScoreType;
    using SolutionType = ProblemType::SolutionType;

  private:
    std::shared_ptr<const ProblemType> problem_;

  protected:
    const ProblemType &getProblem() const { return *problem_; }

  public:
    /// @brief Keeps shared ownership of the problem.
    /// @throws std::invalid_argument if problem is null.
    explicit Optimizer(std::shared_ptr<const ProblemType> problem)
        : problem_(problem ? std::move(problem) : throw std::invalid_argument("Evaluator must not be null")) {}

    /// @brief Executes the configured optimization algorithm.
    virtual void run() = 0;

    /**
     * @brief Returns a copy of the best solution found so far.
     * @warning Not thread-safe. Do not call concurrently with run() on the same instance.
     */
    [[nodiscard]] virtual SolutionType getBest() const = 0;
    virtual ~Optimizer() = default;
};
} // namespace metah
#endif // SRC_METAH_OPTIMIZER_HPP_
