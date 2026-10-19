#ifndef SRC_METAH_OPERATORS_TABU_LIST_HPP_
#define SRC_METAH_OPERATORS_TABU_LIST_HPP_

#include <absl/container/linked_hash_set.h>
#include <cstddef>
#include <functional>
#include <stdexcept>

namespace metah {
template <typename M> class TabuList {
  public:
    using MoveType = M;

    [[nodiscard]] virtual bool contains(const MoveType &move) const = 0;
    virtual void add(const MoveType &move) = 0;
    virtual ~TabuList() = default;
};

template <typename M, typename Hash = absl::DefaultHashContainerHash<M>, typename Eq = absl::DefaultHashContainerEq<M>>
class FifoTabuList final : public TabuList<M> {
  public:
    using MoveType = M;

  private:
    size_t capacity_;
    absl::linked_hash_set<MoveType, Hash, Eq> moves_;

  public:
    explicit FifoTabuList(const size_t capacity) : capacity_(capacity), moves_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("Tabu list capacity must be greater than zero");
        }
        moves_.reserve(capacity);
    }

    [[nodiscard]] bool contains(const MoveType &move) const override { return moves_.contains(move); }

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
