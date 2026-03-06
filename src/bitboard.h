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

    [[nodiscard]] constexpr bool is_empty() const
    {
        return bitboard_ == 0;
    }

    [[nodiscard]] constexpr Bitboard shift(Direction direction) const;

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
        assert(rhs >= 0);
        return Bitboard{bitboard_ << rhs};
    }

    constexpr Bitboard operator>>(const int rhs) const
    {
        assert(rhs >= 0);
        return Bitboard{bitboard_ >> rhs};
    }

    constexpr bool operator==(const Bitboard& rhs) const
    {
        return bitboard_ == rhs.bitboard_;
    }

    constexpr bool operator!=(const Bitboard& rhs) const
    {
        return bitboard_ != rhs.bitboard_;
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

constexpr Bitboard FILE_A{0x0101010101010101ULL};
constexpr Bitboard FILE_B{0x0202020202020202ULL};
constexpr Bitboard FILE_C{0x0404040404040404ULL};
constexpr Bitboard FILE_D{0x0808080808080808ULL};
constexpr Bitboard FILE_E{0x1010101010101010ULL};
constexpr Bitboard FILE_F{0x2020202020202020ULL};
constexpr Bitboard FILE_G{0x4040404040404040ULL};
constexpr Bitboard FILE_H{0x8080808080808080ULL};

constexpr Bitboard RANK_1{0x00000000000000FFULL};
constexpr Bitboard RANK_2{0x000000000000FF00ULL};
constexpr Bitboard RANK_3{0x0000000000FF0000ULL};
constexpr Bitboard RANK_4{0x00000000FF000000ULL};
constexpr Bitboard RANK_5{0x000000FF00000000ULL};
constexpr Bitboard RANK_6{0x0000FF0000000000ULL};
constexpr Bitboard RANK_7{0x00FF000000000000ULL};
constexpr Bitboard RANK_8{0xFF00000000000000ULL};

constexpr Bitboard Bitboard::shift(const Direction direction) const
{
    Bitboard out = *this;

    switch (direction) {
    case Direction::NORTH:
        out &= ~RANK_8;
        break;

    case Direction::SOUTH:
        out &= ~RANK_1;
        break;

    case Direction::EAST:
        out &= ~FILE_H;
        break;

    case Direction::WEST:
        out &= ~FILE_A;
        break;
    default:;
    }

    if (static_cast<int>(direction) < 0) {
        return out >> -static_cast<int>(direction);
    }

    return out << static_cast<int>(direction);
}

template <PieceType PIECE_TYPE>
std::enable_if_t<!is_pawn_v<PIECE_TYPE>, Bitboard> generate_attacks(Square   src,
                                                                    Bitboard blockers) = delete;

template <>
std::enable_if_t<!is_pawn_v<PieceType::KNIGHT>, Bitboard>
generate_attacks<PieceType::KNIGHT>(Square src, Bitboard blockers);

template <>
std::enable_if_t<!is_pawn_v<PieceType::BISHOP>, Bitboard>
generate_attacks<PieceType::BISHOP>(Square src, Bitboard blockers);

template <>
std::enable_if_t<!is_pawn_v<PieceType::ROOK>, Bitboard>
generate_attacks<PieceType::ROOK>(Square src, Bitboard blockers);

template <>
std::enable_if_t<!is_pawn_v<PieceType::QUEEN>, Bitboard>
generate_attacks<PieceType::QUEEN>(Square src, Bitboard blockers);

Bitboard generate_pawn_attacks(Color color, Square src, Bitboard blockers);

} // namespace shellac

#endif // SHELLAC_BITBOARD_H
