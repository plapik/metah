#ifndef SRC_METAH_OPERATORS_GENERATOR_HPP_
#define SRC_METAH_OPERATORS_GENERATOR_HPP_

#include <memory>

namespace metah {
/// @brief Creates initial representations and evaluated solutions for a problem.
template <typename P> class Generator {
  public:
    using ProblemType = P;
    using ReprType = P::ReprType;
    using ScoreType = P::ScoreType;
    using SolutionType = P::SolutionType;

    /// @brief Returns a new representation satisfying the problem's constraints.
    [[nodiscard]] virtual ReprType generateRepr(const ProblemType &problem) = 0;

    /// @brief Generates a representation and evaluates its score using the supplied problem.
    [[nodiscard]] virtual SolutionType generateSolution(const ProblemType &problem) {
        auto repr = generateRepr(problem);
        auto score = problem.evaluate(repr);
        return {repr, score};
    }
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<Generator> clone() const = 0;
    virtual ~Generator() = default;
};
} // namespace metah

#endif // SRC_METAH_OPERATORS_GENERATOR_HPP_
