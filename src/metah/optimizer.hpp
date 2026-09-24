#ifndef SRC_METAH_OPTIMIZER_HPP_
#define SRC_METAH_OPTIMIZER_HPP_

#include <memory>
#include <utility>

namespace metah {
template <typename P>
class Optimizer {
 public:
  using ProblemType = P;
  using ReprType = ProblemType::ReprType;
  using ScoreType = ProblemType::ScoreType;
  using SolutionType = ProblemType::SolutionType;

 private:
  std::shared_ptr<const ProblemType> problem_;

 protected:
  const ProblemType& getProblem() const { return *problem_; }

 public:
  explicit Optimizer(std::shared_ptr<const ProblemType> problem)
      : problem_(problem ? std::move(problem)
                        : throw std::invalid_argument(
                              "Evaluator must not be null")) {}

  virtual void run() = 0;
  [[nodiscard]] virtual SolutionType getBest() const = 0;
  virtual ~Optimizer() = default;
};
}  // namespace metah
#endif  // SRC_METAH_OPTIMIZER_HPP_
