#ifndef SRC_METAH_OPERATORS_BINARY_FLIP_NBR_HPP_
#define SRC_METAH_OPERATORS_BINARY_FLIP_NBR_HPP_

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "metah/operators/neighbour_gen.hpp"
#include "metah/util/rng.hpp"

namespace metah::binary {
/// @brief Zero-based index of a flipped bit.
/// @ingroup binary_operators
struct FlipMove {
    size_t index;

    friend bool operator==(const FlipMove &, const FlipMove &) = default;

    template <typename H> friend H AbslHashValue(H h, const FlipMove &move) {
        return H::combine(std::move(h), move.index);
    }
};

/// @brief Flipped indices, recorded in ascending order by FlipNeighbour; may be empty.
/// @ingroup binary_operators
struct ChanceFlipMove {
    std::vector<size_t> indices;

    friend bool operator==(const ChanceFlipMove &, const ChanceFlipMove &) = default;

    template <typename H> friend H AbslHashValue(H h, const ChanceFlipMove &move) {
        for (const size_t index : move.indices) {
            h = H::combine(std::move(h), index);
        }
        return h;
    }
};

/**
 * @brief Samples neighbors that differ from the original by exactly one random bit.
 * @ingroup binary_operators
 */
template <typename R = std::vector<uint8_t>, typename RNG = ThreadLocalRandom>
class SingleFlipNeighbour final : public NeighbourGen<R, FlipMove> {
    [[no_unique_address]] RNG rng_;
    size_t neighbourhood_size_;

  public:
    using Base = NeighbourGen<R, FlipMove>;
    using typename Base::MoveType;
    using typename Base::ReprType;

    /// @brief Sets the number of random candidates sampled per neighborhood evaluation.
    explicit SingleFlipNeighbour(const size_t neighbourhood_size, RNG rng = RNG())
        : rng_(std::move(rng)), neighbourhood_size_(neighbourhood_size) {}

    /// @brief Returns the configured candidate count, independent of original.
    [[nodiscard]] size_t size(const ReprType &original) const override { return neighbourhood_size_; }

    /// @brief Copies original, flips one bit in target, and returns its index; idx is ignored.
    MoveType generate(const ReprType &original, ReprType &target, size_t idx) override {
        assert(&original != &target);
        assert(!original.empty());
        assert(std::size(target) == std::size(original));
        target = original;
        const size_t bit_index = rng_.randomIdx(std::size(original) - 1);
        target[bit_index] = target[bit_index] ^ 1;
        return {bit_index};
    }

    [[nodiscard]] std::unique_ptr<NeighbourGen<ReprType, MoveType>> clone() const override {
        return std::make_unique<SingleFlipNeighbour>(*this);
    }
};

/**
 * @brief Samples neighbors by independently flipping each bit with a given probability.
 * @ingroup binary_operators
 */
template <typename R = std::vector<uint8_t>, typename RNG = ThreadLocalRandom>
class FlipNeighbour final : public NeighbourGen<R, ChanceFlipMove> {
    [[no_unique_address]] RNG rng_;
    size_t neighbourhood_size_;
    double flip_chance_;

  public:
    using Base = NeighbourGen<R, ChanceFlipMove>;
    using typename Base::MoveType;
    using typename Base::ReprType;

    /// @brief Sets the candidate count and per-bit flip probability.
    explicit FlipNeighbour(const size_t neighbourhood_size, const double flip_chance, RNG rng = RNG())
        : rng_(std::move(rng)), neighbourhood_size_(neighbourhood_size), flip_chance_(flip_chance) {}

    /// @brief Returns the configured candidate count, independent of original.
    [[nodiscard]] size_t size(const ReprType &original) const override { return neighbourhood_size_; }

    /// @brief Copies original, flips selected bits in target, and returns their indices; idx is ignored.
    MoveType generate(const ReprType &original, ReprType &target, size_t idx) override {
        assert(&original != &target);
        assert(!original.empty());
        assert(std::size(target) == std::size(original));
        target = original;
        MoveType move;
        const size_t n = std::size(original);
        move.indices.reserve(n);

        if (flip_chance_ <= 0.0) {
            return move;
        }

        if (flip_chance_ >= 1.0) {
            for (size_t i = 0; i < n; ++i) {
                target[i] = target[i] ^ 1;
                move.indices.push_back(i);
            }
            return move;
        }

        size_t i = 0;
        while (i < n) {
            const size_t gap = rng_.randomGeometricGap(flip_chance_);
            i += gap;
            if (i >= n) {
                break;
            }
            target[i] = target[i] ^ 1;
            move.indices.push_back(i);
            ++i;
        }
        return move;
    }

    [[nodiscard]] std::unique_ptr<NeighbourGen<ReprType, MoveType>> clone() const override {
        return std::make_unique<FlipNeighbour>(*this);
    }
};
} // namespace metah::binary

#endif // SRC_METAH_OPERATORS_BINARY_FLIP_NBR_HPP_
