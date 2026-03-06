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

#include "bitboard.h"

#include <array>

namespace shellac {
template <>
std::enable_if_t<!is_pawn_v<PieceType::KNIGHT>, Bitboard>
generate_attacks<PieceType::KNIGHT>(const Square src, Bitboard)
{
    const auto origin = Bitboard{src};
    Bitboard   out    = Bitboard{};
    out |= origin.shift(Direction::NORTH).shift(Direction::NORTH).shift(Direction::EAST);
    out |= origin.shift(Direction::NORTH).shift(Direction::NORTH).shift(Direction::WEST);
    out |= origin.shift(Direction::EAST).shift(Direction::EAST).shift(Direction::NORTH);
    out |= origin.shift(Direction::EAST).shift(Direction::EAST).shift(Direction::SOUTH);
    out |= origin.shift(Direction::SOUTH).shift(Direction::SOUTH).shift(Direction::EAST);
    out |= origin.shift(Direction::SOUTH).shift(Direction::SOUTH).shift(Direction::WEST);
    out |= origin.shift(Direction::WEST).shift(Direction::WEST).shift(Direction::NORTH);
    out |= origin.shift(Direction::WEST).shift(Direction::WEST).shift(Direction::SOUTH);
    return out;
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::BISHOP>, Bitboard>
generate_attacks<PieceType::BISHOP>(const Square src, const Bitboard blockers)
{
    constexpr std::array<std::pair<Direction, Direction>, 4> OFFSETS = {
        std::pair{Direction::NORTH, Direction::EAST},
        {Direction::NORTH, Direction::WEST},
        {Direction::SOUTH, Direction::EAST},
        {Direction::SOUTH, Direction::WEST},
    };

    Bitboard out{};

    for (const auto [a, b] : OFFSETS) {
        auto scanner = Bitboard{src};
        while (!scanner.is_empty()) {
            scanner = scanner.shift(a).shift(b);
            out |= scanner;
            if (!(scanner & blockers).is_empty()) {
                break;
            }
        }
    }

    return out;
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::ROOK>, Bitboard>
generate_attacks<PieceType::ROOK>(const Square src, const Bitboard blockers)
{
    constexpr std::array<Direction, 4> OFFSETS = {
        Direction::NORTH,
        Direction::EAST,
        Direction::SOUTH,
        Direction::WEST,
    };

    Bitboard out{};

    for (const Direction direction : OFFSETS) {
        auto scanner = Bitboard{src};
        while (!scanner.is_empty()) {
            scanner = scanner.shift(direction);
            out |= scanner;
            if (!(scanner & blockers).is_empty()) {
                break;
            }
        }
    }

    return out;
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::QUEEN>, Bitboard>
generate_attacks<PieceType::QUEEN>(const Square src, const Bitboard blockers)
{
    const Bitboard orthogonalAttacks = generate_attacks<PieceType::ROOK>(src, blockers);
    const Bitboard diagonalAttacks   = generate_attacks<PieceType::BISHOP>(src, blockers);
    return orthogonalAttacks | diagonalAttacks;
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::KING>, Bitboard>
generate_attacks<PieceType::KING>(const Square src, const Bitboard)
{
    const Bitboard origin = Bitboard{src};
    Bitboard       out{};

    out |= origin.shift(Direction::NORTH);
    out |= origin.shift(Direction::SOUTH);
    out |= origin.shift(Direction::EAST);
    out |= origin.shift(Direction::WEST);
    out |= origin.shift(Direction::NORTH).shift(Direction::EAST);
    out |= origin.shift(Direction::NORTH).shift(Direction::WEST);
    out |= origin.shift(Direction::SOUTH).shift(Direction::EAST);
    out |= origin.shift(Direction::SOUTH).shift(Direction::WEST);

    return out;
}

Bitboard generate_pawn_destinations(const Color color, const Square src, const Bitboard blockers)
{
    const Rank      homeRank  = color == Color::WHITE ? Rank::R_2 : Rank::R_7;
    const Direction direction = color == Color::WHITE ? Direction::NORTH : Direction::SOUTH;
    Bitboard        out       = generate_pawn_attacks(color, src, blockers);

    const Bitboard oneAhead = Bitboard{src}.shift(direction);
    if ((blockers & oneAhead).is_empty()) {
        out |= oneAhead;

        if (rank_of(src) == homeRank) {
            const Bitboard twoAhead = oneAhead.shift(direction);
            if ((blockers & twoAhead).is_empty()) {
                out |= twoAhead;
            }
        }
    }

    return out;
}

Bitboard generate_pawn_attacks(const Color color, const Square src, const Bitboard blockers)
{
    const Direction direction = color == Color::WHITE ? Direction::NORTH : Direction::SOUTH;
    const Bitboard  lhs       = Bitboard{src}.shift(direction).shift(Direction::WEST);
    const Bitboard  rhs       = Bitboard{src}.shift(direction).shift(Direction::EAST);

    return (lhs | rhs) & blockers;
}
} // namespace shellac
