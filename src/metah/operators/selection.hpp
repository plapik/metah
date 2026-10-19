#ifndef SRC_METAH_OPERATORS_SELECTION_HPP_
#define SRC_METAH_OPERATORS_SELECTION_HPP_

#include <memory>
#include <span>

namespace metah {
/**
 * @brief Chooses a population member using its score.
 * @note Implementations used by parallel optimizers must support concurrent calls.
 */
template <typename S> class Selection {
  public:
    using ScoreType = S;
    /// @brief Returns an index in [0, scores.size()); scores must not be empty.
    virtual size_t select(std::span<const ScoreType> scores) = 0;
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<Selection> clone() const = 0;
    virtual ~Selection() = default;
};
} // namespace metah

#endif // SRC_METAH_OPERATORS_SELECTION_HPP_
