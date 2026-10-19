#ifndef SRC_METAH_OPERATORS_CROSSOVER_HPP_
#define SRC_METAH_OPERATORS_CROSSOVER_HPP_

#include <memory>

namespace metah {
/**
 * @brief Produces two offspring from two parent representations.
 * @note Implementations used by parallel optimizers must support concurrent calls.
 */
template <typename R> class Crossover {
  public:
    using ReprType = R;
    /**
     * @brief Writes offspring and returns whether they may differ from their respective parents.
     * @pre c1 and c2 are initialized, valid output representations with the required
     * size (e.g. vectors already resized to match the parents), without aliasing
     * either parent or each other. Operators need not allocate or resize outputs.
     * @note If crossover is skipped, outputs must still receive their respective parents.
     */
    virtual bool cross(const ReprType &p1, const ReprType &p2, ReprType &c1, ReprType &c2) = 0;
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<Crossover> clone() const = 0;
    virtual ~Crossover() = default;
};
} // namespace metah

#endif // SRC_METAH_OPERATORS_CROSSOVER_HPP_
