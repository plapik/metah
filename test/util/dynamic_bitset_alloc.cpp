#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <doctest/doctest.h>

#include <metah/util/dynamic_bitset.hpp>

namespace {
struct AllocationState {
    bool fail_allocation = false;
    bool invalid_deallocation = false;
    std::unordered_map<void *, size_t> allocations;
};

template <typename T, bool CopyPropagation = false, bool MovePropagation = false> struct TrackingAllocator {
    using value_type = T;
    using propagate_on_container_copy_assignment = std::bool_constant<CopyPropagation>;
    using propagate_on_container_move_assignment = std::bool_constant<MovePropagation>;
    using is_always_equal = std::false_type;

    template <typename U> struct rebind {
        using other = TrackingAllocator<U, CopyPropagation, MovePropagation>;
    };

    std::shared_ptr<AllocationState> state;

    explicit TrackingAllocator(std::shared_ptr<AllocationState> allocation_state = std::make_shared<AllocationState>())
        : state(std::move(allocation_state)) {}

    TrackingAllocator(const TrackingAllocator &) noexcept = default;
    TrackingAllocator &operator=(const TrackingAllocator &) noexcept = default;
    TrackingAllocator(TrackingAllocator &&other) noexcept : state(other.state) {}
    TrackingAllocator &operator=(TrackingAllocator &&other) noexcept {
        state = other.state;
        return *this;
    }

    template <typename U>
    TrackingAllocator(const TrackingAllocator<U, CopyPropagation, MovePropagation> &other) noexcept
        : state(other.state) {}

    T *allocate(const size_t count) {
        if (state->fail_allocation) {
            throw std::bad_alloc();
        }
        T *data = std::allocator<T>{}.allocate(count);
        try {
            state->allocations.emplace(data, count);
        } catch (...) {
            std::allocator<T>{}.deallocate(data, count);
            throw;
        }
        return data;
    }

    void deallocate(T *data, const size_t count) noexcept {
        const auto found = state->allocations.find(data);
        if (found == state->allocations.end()) {
            state->invalid_deallocation = true;
            return;
        }
        if (found->second != count) {
            state->invalid_deallocation = true;
        }
        std::allocator<T>{}.deallocate(data, found->second);
        state->allocations.erase(found);
    }

    template <typename U>
    bool operator==(const TrackingAllocator<U, CopyPropagation, MovePropagation> &other) const noexcept {
        return state == other.state;
    }
};

using RetainedAllocator = TrackingAllocator<uint64_t>;
using CopyingAllocator = TrackingAllocator<uint64_t, true>;
using MovingAllocator = TrackingAllocator<uint64_t, false, true>;
} // namespace

TEST_SUITE_BEGIN("Dynamic bitset allocator tests");

TEST_CASE_TEMPLATE("Copy assignment preserves contents on allocation failure", Allocator, RetainedAllocator,
                   CopyingAllocator) {
    using Bitset = metah::DynamicBitset<uint64_t, Allocator>;
    auto source_state = std::make_shared<AllocationState>();
    auto target_state = std::make_shared<AllocationState>();
    auto replacement_state = Allocator::propagate_on_container_copy_assignment::value ? source_state : target_state;
    {
        Bitset source(130, Allocator(source_state));
        source.set(129);
        Bitset target(65, Allocator(target_state));
        target.set(64);
        auto *source_data = source.data();
        auto *target_data = target.data();

        replacement_state->fail_allocation = true;
        CHECK_THROWS_AS(target = source, std::bad_alloc);
        replacement_state->fail_allocation = false;

        REQUIRE(target_state->allocations.contains(target_data));
        CHECK(target.data() == target_data);
        CHECK(target.size() == 65);
        CHECK(target.num_blocks() == 2);
        CHECK(target.count() == 1);
        CHECK(target.test(64));
        CHECK(source.data() == source_data);
        CHECK(source.size() == 130);
        CHECK(source.test(129));

        target = source;
        CHECK(target == source);
        CHECK(target.data() != source_data);
        CHECK(replacement_state->allocations.contains(target.data()));
        target.reset(129);
        CHECK(source.test(129));
    }
    CHECK(source_state->allocations.empty());
    CHECK(target_state->allocations.empty());
    CHECK_FALSE(source_state->invalid_deallocation);
    CHECK_FALSE(target_state->invalid_deallocation);
}

TEST_CASE("Move assignment between unequal allocators preserves contents on allocation failure") {
    using Bitset = metah::DynamicBitset<uint64_t, RetainedAllocator>;
    static_assert(!std::is_nothrow_move_assignable_v<Bitset>);
    auto source_state = std::make_shared<AllocationState>();
    auto target_state = std::make_shared<AllocationState>();
    {
        Bitset source(130, RetainedAllocator(source_state));
        source.set(129);
        Bitset target(65, RetainedAllocator(target_state));
        target.set(64);
        auto *source_data = source.data();
        auto *target_data = target.data();

        target_state->fail_allocation = true;
        CHECK_THROWS_AS(target = std::move(source), std::bad_alloc);
        target_state->fail_allocation = false;

        REQUIRE(target_state->allocations.contains(target_data));
        REQUIRE(source_state->allocations.contains(source_data));
        CHECK(target.data() == target_data);
        CHECK(target.size() == 65);
        CHECK(target.test(64));
        CHECK(source.data() == source_data);
        CHECK(source.size() == 130);
        CHECK(source.test(129));

        target = std::move(source);
        CHECK(target.size() == 130);
        CHECK(target.test(129));
        CHECK(target.data() != source_data);
        CHECK(target_state->allocations.contains(target.data()));
        CHECK(source.size() == 0);
        CHECK(source.num_blocks() == 0);
        CHECK(source.data() == nullptr);
        CHECK(source_state->allocations.empty());
    }
    CHECK(target_state->allocations.empty());
    CHECK_FALSE(source_state->invalid_deallocation);
    CHECK_FALSE(target_state->invalid_deallocation);
}

TEST_CASE_TEMPLATE("Move assignment transfers storage when allocators permit", Allocator, RetainedAllocator,
                   MovingAllocator) {
    using Bitset = metah::DynamicBitset<uint64_t, Allocator>;
    auto source_state = std::make_shared<AllocationState>();
    auto target_state =
        Allocator::propagate_on_container_move_assignment::value ? std::make_shared<AllocationState>() : source_state;
    {
        Bitset source(130, Allocator(source_state));
        source.set(129);
        Bitset target(65, Allocator(target_state));
        auto *source_data = source.data();
        auto *target_data = target.data();
        source_state->fail_allocation = true;
        target_state->fail_allocation = true;

        target = std::move(source);
        CHECK(target.data() == source_data);
        CHECK(target.size() == 130);
        CHECK(target.test(129));
        CHECK_FALSE(target_state->allocations.contains(target_data));
        CHECK(source.size() == 0);
        CHECK(source.num_blocks() == 0);
        CHECK(source.data() == nullptr);
    }
    CHECK(source_state->allocations.empty());
    CHECK(target_state->allocations.empty());
    CHECK_FALSE(source_state->invalid_deallocation);
    CHECK_FALSE(target_state->invalid_deallocation);
}

TEST_CASE_TEMPLATE("Copy assignment from an empty bitset needs no allocation", Allocator, RetainedAllocator,
                   CopyingAllocator) {
    using Bitset = metah::DynamicBitset<uint64_t, Allocator>;
    auto source_state = std::make_shared<AllocationState>();
    auto target_state = std::make_shared<AllocationState>();
    {
        Bitset source(0, Allocator(source_state));
        Bitset target(65, Allocator(target_state));
        source_state->fail_allocation = true;
        target_state->fail_allocation = true;

        target = source;
        CHECK(target.size() == 0);
        CHECK(target.num_blocks() == 0);
        CHECK(target.data() == nullptr);
        CHECK(target_state->allocations.empty());
    }
    CHECK(source_state->allocations.empty());
    CHECK_FALSE(target_state->invalid_deallocation);
}

TEST_CASE("Bitset construction handles allocation failure") {
    using Bitset = metah::DynamicBitset<uint64_t, RetainedAllocator>;
    auto state = std::make_shared<AllocationState>();
    {
        Bitset source(130, RetainedAllocator(state));
        source.set(129);
        state->fail_allocation = true;

        CHECK_THROWS_AS(static_cast<void>(Bitset(65, RetainedAllocator(state))), std::bad_alloc);
        CHECK_THROWS_AS(static_cast<void>(Bitset(source)), std::bad_alloc);
        CHECK(state->allocations.size() == 1);
        CHECK(source.size() == 130);
        CHECK(source.test(129));
    }
    CHECK(state->allocations.empty());
    CHECK_FALSE(state->invalid_deallocation);
}

TEST_CASE_TEMPLATE("Self assignment preserves storage without allocating", Allocator, RetainedAllocator,
                   CopyingAllocator, MovingAllocator) {
    using Bitset = metah::DynamicBitset<uint64_t, Allocator>;
    auto state = std::make_shared<AllocationState>();
    {
        Bitset bitset(130, Allocator(state));
        bitset.set(129);
        auto *data = bitset.data();
        auto &alias = bitset;
        state->fail_allocation = true;

        bitset = alias;
        bitset = std::move(alias);
        CHECK(bitset.data() == data);
        CHECK(bitset.size() == 130);
        CHECK(bitset.test(129));
    }
    CHECK(state->allocations.empty());
    CHECK_FALSE(state->invalid_deallocation);
}

TEST_CASE("Default allocator preserves noexcept moves") {
    static_assert(std::is_nothrow_move_constructible_v<metah::DynamicBitset<>>);
    static_assert(std::is_nothrow_move_assignable_v<metah::DynamicBitset<>>);
    static_assert(std::is_nothrow_move_assignable_v<metah::DynamicBitset<uint64_t, MovingAllocator>>);
}

TEST_SUITE_END();
