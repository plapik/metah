#ifndef SRC_METAH_UTIL_DYNAMIC_BITSET_HPP_
#define SRC_METAH_UTIL_DYNAMIC_BITSET_HPP_

#include <algorithm>
#include <bit>
#include <cassert>
#include <climits>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

// Experimental, implementation partially made by AI

namespace metah {
/**
 * @brief A bit sequence with a size chosen at construction and independent copy storage.
 * @tparam Block Unsigned integer storage type with a power-of-two bit width.
 * @tparam Allocator Allocator for Block values using raw pointers.
 *
 * Bit indices must be below size(); nonempty ranges must fit within size().
 * Bounds are checked by assertions only. Bit zero is the least significant bit
 * of the first block.
 *
 * @note Concurrent reads are safe without concurrent writes. Modifications to
 * the same instance require synchronization, even when accessing different bits.
 */
template <typename Block = uint64_t, typename Allocator = std::allocator<Block>> class DynamicBitset {
    static_assert(std::is_unsigned_v<Block>, "Block type must be an unsigned integer");

  public:
    static constexpr size_t BITS_PER_BLOCK = sizeof(Block) * CHAR_BIT;
    static_assert(std::has_single_bit(BITS_PER_BLOCK), "Block bit size must be a power of two");

    static constexpr size_t BLOCK_SHIFT = std::countr_zero(BITS_PER_BLOCK);
    static constexpr size_t BLOCK_MASK = BITS_PER_BLOCK - 1;

    static constexpr size_t blockIndex(const size_t pos) noexcept { return pos >> BLOCK_SHIFT; }
    static constexpr size_t bitIndex(const size_t pos) noexcept { return pos & BLOCK_MASK; }
    static constexpr size_t calcNumBlocks(const size_t bits) noexcept { return (bits + BLOCK_MASK) >> BLOCK_SHIFT; }

    /// @brief Returns the number of bits in a partial final block, or zero for full blocks.
    [[nodiscard]] size_t lastBlockBits() const noexcept { return num_bits_ & BLOCK_MASK; }

    /// @brief Clears unused bits in the final storage block.
    void sanitize() noexcept {
        size_t extra = lastBlockBits();
        if (extra > 0 && num_blocks_ > 0) {
            blocks_[num_blocks_ - 1] &= (Block(1) << extra) - 1;
        }
    }

  private:
    using AllocTraits = std::allocator_traits<Allocator>;

    Allocator alloc_;
    size_t num_bits_;
    size_t num_blocks_;
    Block *blocks_;

    void release() noexcept {
        if (blocks_) {
            AllocTraits::deallocate(alloc_, blocks_, num_blocks_);
        }
        num_bits_ = 0;
        num_blocks_ = 0;
        blocks_ = nullptr;
    }

    void swapStorage(DynamicBitset &other) noexcept {
        using std::swap;
        swap(num_bits_, other.num_bits_);
        swap(num_blocks_, other.num_blocks_);
        swap(blocks_, other.blocks_);
    }

    DynamicBitset(const DynamicBitset &other, const Allocator &alloc)
        : alloc_(alloc), num_bits_(other.num_bits_), num_blocks_(other.num_blocks_), blocks_(nullptr) {
        if (num_blocks_ > 0) {
            blocks_ = AllocTraits::allocate(alloc_, num_blocks_);
            std::copy_n(other.blocks_, num_blocks_, blocks_);
        }
    }

  public:
    /// @brief Writable bit proxy; accesses the bitset rather than owning a separate value.
    class reference {
        DynamicBitset &bitset_;
        size_t pos_;

      public:
        reference(DynamicBitset &bitset, const size_t pos) noexcept : bitset_(bitset), pos_(pos) {}
        reference &operator=(const bool v) noexcept {
            if (v) {
                bitset_.set(pos_);
            } else {
                bitset_.reset(pos_);
            }
            return *this;
        }
        reference &operator=(const reference &v) noexcept { return *this = static_cast<bool>(v); }

        operator bool() const noexcept { return bitset_.test(pos_); }
        bool operator~() const noexcept { return !bitset_.test(pos_); }
        reference &flip() noexcept {
            bitset_.flip(pos_);
            return *this;
        }
    };

    /// @brief Creates a zero-initialized bitset of the requested size; allocation failures propagate.
    explicit DynamicBitset(const size_t bits, const Allocator &alloc = Allocator())
        : alloc_(alloc), num_bits_(bits), num_blocks_(calcNumBlocks(bits)), blocks_(nullptr) {
        if (num_blocks_ > 0) {
            blocks_ = AllocTraits::allocate(alloc_, num_blocks_);
            std::fill_n(blocks_, num_blocks_, Block(0));
        }
    }

    ~DynamicBitset() { release(); }

    /// @brief Deep-copies using the allocator selected for copy construction.
    DynamicBitset(const DynamicBitset &other)
        : DynamicBitset(other, AllocTraits::select_on_container_copy_construction(other.alloc_)) {}

    /// @brief Transfers storage and leaves other empty.
    DynamicBitset(DynamicBitset &&other) noexcept(std::is_nothrow_move_constructible_v<Allocator>)
        : alloc_(std::move(other.alloc_)), num_bits_(std::exchange(other.num_bits_, 0)),
          num_blocks_(std::exchange(other.num_blocks_, 0)), blocks_(std::exchange(other.blocks_, nullptr)) {}

    /// @brief Deep-copies with allocator propagation; allocation failure leaves both operands unchanged.
    DynamicBitset &operator=(const DynamicBitset &other) {
        if (this != &other) {
            if constexpr (AllocTraits::propagate_on_container_copy_assignment::value) {
                DynamicBitset replacement(other, other.alloc_);
                release();
                alloc_ = other.alloc_;
                swapStorage(replacement);
            } else {
                DynamicBitset replacement(other, alloc_);
                swapStorage(replacement);
            }
        }
        return *this;
    }

    /**
     * @brief Moves the value and leaves other empty unless assigning to itself.
     * Unequal non-propagating allocators require allocation, whose failure leaves
     * both operands unchanged.
     */
    DynamicBitset &operator=(DynamicBitset &&other) noexcept(AllocTraits::propagate_on_container_move_assignment::value
                                                                 ? std::is_nothrow_move_assignable_v<Allocator>
                                                                 : AllocTraits::is_always_equal::value) {
        if (this != &other) {
            if constexpr (AllocTraits::propagate_on_container_move_assignment::value) {
                release();
                alloc_ = std::move(other.alloc_);
                swapStorage(other);
            } else if constexpr (AllocTraits::is_always_equal::value) {
                release();
                swapStorage(other);
            } else if (alloc_ == other.alloc_) {
                release();
                swapStorage(other);
            } else {
                DynamicBitset replacement(other, alloc_);
                swapStorage(replacement);
                other.release();
            }
        }
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept { return num_bits_; }
    [[nodiscard]] size_t num_blocks() const noexcept { return num_blocks_; }

    /// @brief Returns mutable block storage. Restore padding bits with sanitize() after raw writes.
    Block *data() noexcept { return blocks_; }
    /// @brief Returns read-only block storage. Assignment or moving storage may invalidate the pointer.
    const Block *data() const noexcept { return blocks_; }

    /// @brief Reads the bit at pos; requires pos < size().
    [[nodiscard]] bool test(const size_t pos) const noexcept {
        assert(pos < num_bits_ && "Bit position out of range");
        return blocks_[blockIndex(pos)] >> bitIndex(pos) & 1;
    }

    void set(const size_t pos) noexcept {
        assert(pos < num_bits_ && "Bit position out of range");
        blocks_[blockIndex(pos)] |= Block(1) << bitIndex(pos);
    }

    void reset(const size_t pos) noexcept {
        assert(pos < num_bits_ && "Bit position out of range");
        blocks_[blockIndex(pos)] &= ~(Block(1) << bitIndex(pos));
    }

    void flip(const size_t pos) noexcept {
        assert(pos < num_bits_ && "Bit position out of range");
        blocks_[blockIndex(pos)] ^= Block(1) << bitIndex(pos);
    }

    /// @brief Sets the length bits starting at startpos; a zero length does nothing.
    void set(const size_t startpos, const size_t length) noexcept {
        if (length == 0) {
            return;
        }
        assert(startpos + length <= num_bits_ && "Range out of bounds");

        size_t first_block = blockIndex(startpos);
        size_t last_block = blockIndex(startpos + length - 1);
        size_t first_bit = bitIndex(startpos);
        const size_t last_bit = bitIndex(startpos + length - 1);

        Block first_mask = ~Block(0) << first_bit;
        Block last_mask = last_bit == BLOCK_MASK ? ~Block(0) : (Block(1) << (last_bit + 1)) - 1;

        if (first_block == last_block) {
            blocks_[first_block] |= first_mask & last_mask;
        } else {
            blocks_[first_block] |= first_mask;
            if (last_block > first_block + 1) {
                std::fill_n(blocks_ + first_block + 1, last_block - first_block - 1, ~Block(0));
            }
            blocks_[last_block] |= last_mask;
        }
    }

    /// @brief Clears the length bits starting at startpos; a zero length does nothing.
    void reset(const size_t startpos, const size_t length) noexcept {
        if (length == 0) {
            return;
        }
        assert(startpos + length <= num_bits_ && "Range out of bounds");

        size_t first_block = blockIndex(startpos);
        size_t last_block = blockIndex(startpos + length - 1);
        size_t first_bit = bitIndex(startpos);
        const size_t last_bit = bitIndex(startpos + length - 1);

        Block first_mask = ~Block(0) << first_bit;
        Block last_mask = last_bit == BLOCK_MASK ? ~Block(0) : (Block(1) << (last_bit + 1)) - 1;

        if (first_block == last_block) {
            blocks_[first_block] &= ~(first_mask & last_mask);
        } else {
            blocks_[first_block] &= ~first_mask;
            if (last_block > first_block + 1) {
                std::fill_n(blocks_ + first_block + 1, last_block - first_block - 1, Block(0));
            }
            blocks_[last_block] &= ~last_mask;
        }
    }

    /// @brief Toggles the length bits starting at startpos; a zero length does nothing.
    void flip(const size_t startpos, const size_t length) noexcept {
        if (length == 0) {
            return;
        }
        assert(startpos + length <= num_bits_ && "Range out of bounds");

        const size_t first_block = blockIndex(startpos);
        const size_t last_block = blockIndex(startpos + length - 1);

        const Block first_mask = ~Block(0) << bitIndex(startpos);
        const Block last_mask = ~((~Block(0) << 1) << bitIndex(startpos + length - 1));
        const Block same_mask = -static_cast<Block>(first_block == last_block);
        const Block diff_mask = ~same_mask;

        Block *const data = blocks_;

        data[first_block] ^= first_mask & (last_mask | diff_mask);
        data[last_block] ^= last_mask & diff_mask;

        for (size_t i = first_block + 1; i < last_block; ++i) {
            data[i] = ~data[i];
        }
    }

    void set() noexcept {
        std::fill_n(blocks_, num_blocks_, ~Block(0));
        sanitize();
    }
    void reset() noexcept { std::fill_n(blocks_, num_blocks_, Block(0)); }
    void flip() noexcept {
        for (size_t i = 0; i < num_blocks_; ++i) {
            blocks_[i] = ~blocks_[i];
        }
        sanitize();
    }

    /// @brief Counts set bits in O(num_blocks()) time.
    [[nodiscard]] size_t count() const noexcept {
        size_t c = 0;
        for (size_t i = 0; i < num_blocks_; ++i) {
            c += std::popcount(blocks_[i]);
        }
        return c;
    }

    /// @brief Returns whether every bit is set; true for an empty bitset.
    [[nodiscard]] bool all() const noexcept {
        if (num_bits_ == 0) {
            return true;
        }
        size_t extra = lastBlockBits();
        const size_t full_blocks = num_blocks_ - (extra > 0 ? 1 : 0);

        for (size_t i = 0; i < full_blocks; ++i) {
            if (blocks_[i] != ~Block(0)) {
                return false;
            }
        }
        if (extra > 0) {
            Block mask = (Block(1) << extra) - 1;
            if (blocks_[num_blocks_ - 1] != mask) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool any() const noexcept {
        for (size_t i = 0; i < num_blocks_; ++i) {
            if (blocks_[i] != 0) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool none() const noexcept { return !any(); }

    /// @brief Returns a writable bit proxy; requires pos < size().
    reference operator[](size_t pos) noexcept { return reference(*this, pos); }
    bool operator[](const size_t pos) const noexcept { return test(pos); }

    /// @brief ANDs overlapping blocks, leaving unmatched blocks and the size unchanged.
    DynamicBitset &operator&=(const DynamicBitset &other) noexcept {
        const size_t n = std::min(num_blocks_, other.num_blocks_);
        for (size_t i = 0; i < n; ++i) {
            blocks_[i] &= other.blocks_[i];
        }
        return *this;
    }

    /// @brief ORs overlapping blocks, leaving unmatched blocks and the size unchanged.
    DynamicBitset &operator|=(const DynamicBitset &other) noexcept {
        const size_t n = std::min(num_blocks_, other.num_blocks_);
        for (size_t i = 0; i < n; ++i) {
            blocks_[i] |= other.blocks_[i];
        }
        return *this;
    }

    /// @brief XORs overlapping blocks, leaving unmatched blocks and the size unchanged.
    DynamicBitset &operator^=(const DynamicBitset &other) noexcept {
        const size_t n = std::min(num_blocks_, other.num_blocks_);
        for (size_t i = 0; i < n; ++i) {
            blocks_[i] ^= other.blocks_[i];
        }
        return *this;
    }

    DynamicBitset operator~() const {
        DynamicBitset res(*this);
        res.flip();
        return res;
    }

    /// @brief Shifts toward higher indices, filling with zeros; shift >= size() clears all bits.
    DynamicBitset &operator<<=(const size_t shift) noexcept {
        if (shift >= num_bits_) {
            reset();
            return *this;
        }
        if (shift == 0) {
            return *this;
        }

        size_t b_shift = blockIndex(shift);
        size_t bit_shift = bitIndex(shift);

        if (bit_shift == 0) {
            for (size_t i = num_blocks_; i > b_shift; --i) {
                blocks_[i - 1] = blocks_[i - 1 - b_shift];
            }
        } else {
            size_t inv_shift = BITS_PER_BLOCK - bit_shift;
            for (size_t i = num_blocks_; i > b_shift + 1; --i) {
                blocks_[i - 1] = blocks_[i - 1 - b_shift] << bit_shift | blocks_[i - 2 - b_shift] >> inv_shift;
            }
            blocks_[b_shift] = blocks_[0] << bit_shift;
        }

        std::fill_n(blocks_, b_shift, Block(0));
        sanitize();
        return *this;
    }

    /// @brief Shifts toward lower indices, filling with zeros; shift >= size() clears all bits.
    DynamicBitset &operator>>=(const size_t shift) noexcept {
        if (shift >= num_bits_) {
            reset();
            return *this;
        }
        if (shift == 0) {
            return *this;
        }

        size_t b_shift = blockIndex(shift);
        size_t bit_shift = bitIndex(shift);

        if (bit_shift == 0) {
            for (size_t i = 0; i < num_blocks_ - b_shift; ++i) {
                blocks_[i] = blocks_[i + b_shift];
            }
        } else {
            size_t inv_shift = BITS_PER_BLOCK - bit_shift;
            for (size_t i = 0; i < num_blocks_ - b_shift - 1; ++i) {
                blocks_[i] = blocks_[i + b_shift] >> bit_shift | blocks_[i + b_shift + 1] << inv_shift;
            }
            blocks_[num_blocks_ - b_shift - 1] = blocks_[num_blocks_ - 1] >> bit_shift;
        }

        std::fill_n(blocks_ + (num_blocks_ - b_shift), b_shift, Block(0));
        return *this;
    }

    // --- Global Binary Operators ---
    friend DynamicBitset operator&(DynamicBitset lhs, const DynamicBitset &rhs) {
        lhs &= rhs;
        return lhs;
    }
    friend DynamicBitset operator|(DynamicBitset lhs, const DynamicBitset &rhs) {
        lhs |= rhs;
        return lhs;
    }
    friend DynamicBitset operator^(DynamicBitset lhs, const DynamicBitset &rhs) {
        lhs ^= rhs;
        return lhs;
    }
    friend DynamicBitset operator<<(DynamicBitset lhs, size_t shift) {
        lhs <<= shift;
        return lhs;
    }
    friend DynamicBitset operator>>(DynamicBitset lhs, size_t shift) {
        lhs >>= shift;
        return lhs;
    }

    bool operator==(const DynamicBitset &other) const noexcept {
        if (num_bits_ != other.num_bits_) {
            return false;
        }
        return std::equal(blocks_, blocks_ + num_blocks_, other.blocks_);
    }

    bool operator!=(const DynamicBitset &other) const noexcept { return !(*this == other); }
};
} // namespace metah

#endif // SRC_METAH_UTIL_DYNAMIC_BITSET_HPP_
