#ifndef SRC_METAH_OPERATORS_COOLING_SCHEMA_HPP_
#define SRC_METAH_OPERATORS_COOLING_SCHEMA_HPP_

#include <cstddef>
#include <memory>

namespace metah {
class CoolingSchema {
  public:
    [[nodiscard]] virtual double newTemperature(double current_temperature, size_t iteration) const = 0;
    [[nodiscard]] virtual std::unique_ptr<CoolingSchema> clone() const = 0;
    virtual ~CoolingSchema() = default;
};
} // namespace metah

#endif
