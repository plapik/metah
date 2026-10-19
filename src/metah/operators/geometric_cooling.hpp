#ifndef SRC_METAH_OPERATORS_GEOMETRIC_COOLING_HPP_
#define SRC_METAH_OPERATORS_GEOMETRIC_COOLING_HPP_

#include <memory>
#include <stdexcept>

#include "metah/operators/cooling_schema.hpp"

namespace metah {
class GeometricCooling final : public CoolingSchema {
    double alpha_;

  public:
    explicit GeometricCooling(const double alpha) : alpha_(alpha) {
        if (alpha <= 0.0 || alpha >= 1.0) {
            throw std::invalid_argument("Alpha must be strictly between 0.0 "
                                        "and 1.0 for Geometric Cooling.");
        }
    }

    [[nodiscard]] double newTemperature(const double current_temperature, size_t iteration) const override {
        return current_temperature * alpha_;
    }

    [[nodiscard]] std::unique_ptr<CoolingSchema> clone() const override {
        return std::make_unique<GeometricCooling>(*this);
    }
};
} // namespace metah

#endif
