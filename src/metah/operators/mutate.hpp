#ifndef SRC_METAH_OPERATORS_MUTATE_HPP_
#define SRC_METAH_OPERATORS_MUTATE_HPP_

#include <memory>

namespace metah {
template <typename R>
class Mutate {
 public:
  using ReprType = R;
  virtual bool mutate(ReprType& repr) = 0;
  [[nodiscard]] virtual std::unique_ptr<Mutate> clone() const = 0;
  virtual ~Mutate() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_MUTATE_HPP_
