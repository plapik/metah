#ifndef SRC_METAH_OPERATORS_PERTURBATION_HPP_
#define SRC_METAH_OPERATORS_PERTURBATION_HPP_

#include <memory>

namespace metah {
/// @brief Produces a perturbed candidate from the current representation.
template <typename R> class Perturbation {
  public:
    using ReprType = R;
    /**
     * @brief Writes a perturbed representation to target.
     * @pre target is initialized and valid, with the required size (e.g. a vector
     * already resized to match current), and does not alias current.
     * Implementations need not allocate or resize target.
     */
    virtual void perturb(const ReprType &current, ReprType &target) = 0;
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<Perturbation> clone() const = 0;
    virtual ~Perturbation() = default;
};
} // namespace metah

#endif // SRC_METAH_OPERATORS_PERTURBATION_HPP_
