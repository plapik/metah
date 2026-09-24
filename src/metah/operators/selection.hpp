#ifndef SRC_METAH_OPERATORS_SELECTION_HPP_
#define SRC_METAH_OPERATORS_SELECTION_HPP_

#include <memory>
#include <span>

namespace metah {
template <typename S>
class Selection {
 public:
  using ScoreType = S;
  virtual size_t select(std::span<const ScoreType> scores) = 0;
  [[nodiscard]] virtual std::unique_ptr<Selection> clone() const = 0;
  virtual ~Selection() = default;
};
}  // namespace metah

#endif  // SRC_METAH_OPERATORS_SELECTION_HPP_
