#ifndef SRC_METAH_OPERATORS_NEIGHBOUR_GEN_HPP_
#define SRC_METAH_OPERATORS_NEIGHBOUR_GEN_HPP_

#include <memory>

namespace metah {
template <typename R, typename M>
class NeighbourGen {
 public:
  using ReprType = R;
  using MoveType = M;
  [[nodiscard]] virtual size_t size(const ReprType& original) const = 0;
  virtual MoveType generate(const ReprType& original, ReprType& target,
                            size_t idx) = 0;
  [[nodiscard]] virtual std::unique_ptr<NeighbourGen> clone() const = 0;
  virtual ~NeighbourGen() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_NEIGHBOUR_GEN_HPP_
