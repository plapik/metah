#ifndef SRC_METAH_OPERATORS_PERTURBATION_HPP_
#define SRC_METAH_OPERATORS_PERTURBATION_HPP_

#include <memory>

namespace metah {
template <typename R>
class Perturbation {
 public:
  using ReprType = R;
  virtual void perturb(const ReprType& current, ReprType& target) = 0;
  [[nodiscard]] virtual std::unique_ptr<Perturbation> clone() const = 0;
  virtual ~Perturbation() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_PERTURBATION_HPP_
