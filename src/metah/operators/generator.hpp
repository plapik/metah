#ifndef SRC_METAH_OPERATORS_GENERATOR_HPP_
#define SRC_METAH_OPERATORS_GENERATOR_HPP_

#include <memory>

namespace metah {
template <typename P>
class Generator {
 public:
  using ProblemType = P;
  using ReprType = typename P::ReprType;
  using ScoreType = typename P::ScoreType;
  using SolutionType = typename P::SolutionType;

  [[nodiscard]] virtual ReprType generateRepr(const ProblemType& problem) = 0;

  [[nodiscard]] virtual SolutionType generateSolution(
      const ProblemType& problem) {
    auto repr = generateRepr(problem);
    auto score = problem.evaluate(repr);
    return {repr, score};
  }
  [[nodiscard]] virtual std::unique_ptr<Generator> clone() const = 0;
  virtual ~Generator() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_GENERATOR_HPP_
