#ifndef SRC_METAH_OPERATORS_NEIGHBOUR_GEN_HPP_
#define SRC_METAH_OPERATORS_NEIGHBOUR_GEN_HPP_

#include <memory>

namespace metah {
/**
 * @brief Generates neighboring representations and identifies the moves producing them.
 * @note Parallel tabu search may call size() and generate() concurrently on the same instance.
 */
template <typename R, typename M> class NeighbourGen {
  public:
    using ReprType = R;
    using MoveType = M;
    /// @brief Returns the number of candidates to generate for the original representation.
    [[nodiscard]] virtual size_t size(const ReprType &original) const = 0;
    /**
     * @brief Writes a neighbor to target and returns its move.
     * @pre idx < size(original). The target is an initialized, valid output representation
     * with the required size (e.g. a vector already resized to match original) and
     * must not alias original. Implementations need not allocate or resize target.
     */
    virtual MoveType generate(const ReprType &original, ReprType &target, size_t idx) = 0;
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<NeighbourGen> clone() const = 0;
    virtual ~NeighbourGen() = default;
};
} // namespace metah

#endif // SRC_METAH_OPERATORS_NEIGHBOUR_GEN_HPP_
