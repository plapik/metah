#ifndef SRC_METAH_OPERATORS_CROSSOVER_HPP_
#define SRC_METAH_OPERATORS_CROSSOVER_HPP_

#include <memory>

namespace metah {
template <typename R>
class Crossover {
 public:
  using ReprType = R;
  virtual bool cross(const ReprType& p1, const ReprType& p2, ReprType& c1,
                     ReprType& c2) = 0;
  [[nodiscard]] virtual std::unique_ptr<Crossover> clone() const = 0;
  virtual ~Crossover() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_CROSSOVER_HPP_
