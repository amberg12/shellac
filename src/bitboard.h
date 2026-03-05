/*
  Shellac - A UCI chess engine.
  Copyright (C) 2026 Amber Goulding

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SHELLAC_BITBOARD_H
#define SHELLAC_BITBOARD_H
#include <cstdint>

#include "types.h"

namespace shellac {

class Bitboard
{
public:
    constexpr Bitboard() = default;
    explicit constexpr Bitboard(const uint64_t value) : bitboard_(value)
    {
    }
    explicit constexpr Bitboard(const Square value) : bitboard_(1ULL << underlying(value))
    {
    }
    explicit constexpr operator std::uint64_t() const
    {
        return bitboard_;
    }

    constexpr void set_square(const Square square)
    {
        *this |= Bitboard(square);
    }

    constexpr void unset_square(const Square square)
    {
        *this &= ~Bitboard(square);
    }

    [[nodiscard]] constexpr int ctz() const
    {
        return shellac::ctz(bitboard_);
    }

    [[nodiscard]] constexpr Bitboard lsb() const
    {
        return Bitboard(1) << ctz();
    }

    constexpr void pop_lsb()
    {
        *this &= ~lsb();
    }

    [[nodiscard]] constexpr Square lsb_square() const
    {
        return static_cast<Square>(ctz());
    }

    constexpr Bitboard& operator|=(const Bitboard rhs)
    {
        bitboard_ |= rhs.bitboard_;
        return *this;
    }

    constexpr Bitboard operator|(const Bitboard rhs) const
    {
        return Bitboard{bitboard_ | rhs.bitboard_};
    }

    constexpr Bitboard operator&=(const Bitboard rhs)
    {
        bitboard_ &= rhs.bitboard_;
        return *this;
    }

    constexpr Bitboard operator&(const Bitboard rhs) const
    {
        return Bitboard{bitboard_ & rhs.bitboard_};
    }

    constexpr Bitboard operator~() const
    {
        return Bitboard{~bitboard_};
    }

    constexpr Bitboard operator<<(const int rhs) const
    {
        return Bitboard{bitboard_ << rhs};
    }

    struct Iterator
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = ptrdiff_t;
        using value_type        = Square;
        using pointer           = void;
        using reference         = void;

        explicit constexpr Iterator(const std::uint64_t bits) : iter_(bits)
        {
        }

        value_type operator*() const
        {
            return Bitboard(iter_).lsb_square();
        }

        value_type operator->() const
        {
            return Bitboard(iter_).lsb_square();
        }

        Iterator& operator++()
        {
            auto iterBoard = Bitboard(iter_);
            iterBoard.pop_lsb();
            iter_ = static_cast<std::uint64_t>(iterBoard);
            return *this;
        }

        Iterator operator++(int)
        {
            const Iterator temp = *this;
            ++(*this);
            return temp;
        }

        constexpr bool operator==(const Iterator& rhs) const
        {
            return iter_ == rhs.iter_;
        }

        constexpr bool operator!=(const Iterator& rhs) const
        {
            return iter_ != rhs.iter_;
        }

    private:
        std::uint64_t iter_{};
    };

    [[nodiscard]] Iterator begin() const
    {
        return Iterator(bitboard_);
    }

    static Iterator end()
    {
        return Iterator(0);
    }

private:
    std::uint64_t bitboard_{};
};

} // namespace shellac

#endif // SHELLAC_BITBOARD_H
