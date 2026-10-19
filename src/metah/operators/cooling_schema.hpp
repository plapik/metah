#ifndef SRC_METAH_OPERATORS_COOLING_SCHEMA_HPP_
#define SRC_METAH_OPERATORS_COOLING_SCHEMA_HPP_

#include <cstddef>
#include <memory>

namespace metah {
/// @brief Updates the temperature after each simulated annealing iteration.
class CoolingSchema {
  public:
    /// @brief Computes the next temperature from the current value and iteration number.
    [[nodiscard]] virtual double newTemperature(double current_temperature, size_t iteration) const = 0;
    /// @brief Returns an owning copy of this operator.
    [[nodiscard]] virtual std::unique_ptr<CoolingSchema> clone() const = 0;
    virtual ~CoolingSchema() = default;
};
} // namespace metah

#endif
