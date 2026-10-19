#ifndef SRC_METAH_OPERATORS_TABU_LIST_HPP_
#define SRC_METAH_OPERATORS_TABU_LIST_HPP_

#include <absl/container/linked_hash_set.h>
#include <cstddef>
#include <functional>
#include <stdexcept>

namespace metah {
/**
 * @brief Tracks moves excluded from ordinary tabu search candidate selection.
 * @note contains() must support concurrent reads; add() requires exclusive access.
 */
template <typename M> class TabuList {
  public:
    using MoveType = M;

    /// @brief Returns whether the move is currently tabu.
    [[nodiscard]] virtual bool contains(const MoveType &move) const = 0;
    /// @brief Records a move according to the list's retention policy.
    virtual void add(const MoveType &move) = 0;
    virtual ~TabuList() = default;
};

/// @brief Bounded FIFO memory; adding an existing move refreshes it to the queue's end.
template <typename M, typename Hash = absl::DefaultHashContainerHash<M>, typename Eq = absl::DefaultHashContainerEq<M>>
class FifoTabuList final : public TabuList<M> {
  public:
    using MoveType = M;

  private:
    size_t capacity_;
    absl::linked_hash_set<MoveType, Hash, Eq> moves_;

  public:
    /// @brief Creates an empty list with the given positive capacity.
    /// @throws std::invalid_argument if capacity is zero.
    explicit FifoTabuList(const size_t capacity) : capacity_(capacity), moves_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("Tabu list capacity must be greater than zero");
        }
        moves_.reserve(capacity);
    }

    /// @brief Returns whether the move is currently tabu.
    [[nodiscard]] bool contains(const MoveType &move) const override { return moves_.contains(move); }

    /// @brief Refreshes an existing move or inserts a new one, evicting the oldest if full.
    void add(const MoveType &move) override {
        moves_.erase(move);
        if (moves_.size() >= capacity_) {
            moves_.pop_front();
        }
        moves_.insert(move);
    }
};
}

#endif
