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
#include <optional>
#include <random>
#include <vector>

namespace shellac {
namespace {
struct MagicEntry
{
    Bitboard      mask;
    std::uint64_t magic;
    std::uint8_t  indexBits;
};

std::array<MagicEntry, 64> rookMagics;
std::array<MagicEntry, 64> bishopMagics;

std::array<std::vector<Bitboard>, 64> rookMoves;
std::array<std::vector<Bitboard>, 64> bishopMoves;

enum Slider
{
    ROOK,
    BISHOP,
};

inline size_t magic_index(const MagicEntry& magicEntry, Bitboard blockers)
{
    blockers           = blockers & magicEntry.mask;
    std::uint64_t hash = std::uint64_t(blockers) * magicEntry.magic;
    return hash >> (64 - magicEntry.indexBits);
}

inline Bitboard get_rook_moves(Square square, Bitboard blockers)
{
    const MagicEntry& magic = rookMagics[underlying(square)];
    const auto        moves = rookMoves[underlying(square)];
    return moves[magic_index(magic, blockers)];
}

inline Bitboard get_bishop_moves(Square square, Bitboard blockers)
{
    const MagicEntry& magic = bishopMagics[underlying(square)];
    const auto        moves = bishopMoves[underlying(square)];
    return moves[magic_index(magic, blockers)];
}

template <Slider SLIDER>
Bitboard generate_slider_moves(Square src, Bitboard blockers);

template <>
Bitboard generate_slider_moves<Slider::BISHOP>(Square src, Bitboard blockers)
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
Bitboard generate_slider_moves<Slider::ROOK>(Square src, Bitboard blockers)
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

template <Slider SLIDER>
Bitboard relevant_blockers(Square src)
{
    Bitboard moves = generate_slider_moves<SLIDER>(src, Bitboard{});

    Bitboard mask = moves;
    Bitboard srcBB(src);

    if (!srcBB.intersects(RANK_1)) {
        mask &= ~RANK_1;
    }
    if (!srcBB.intersects(RANK_8)) {
        mask &= ~RANK_8;
    }
    if (!srcBB.intersects(FILE_A)) {
        mask &= ~FILE_A;
    }
    if (!srcBB.intersects(FILE_H)) {
        mask &= ~FILE_H;
    }

    return mask;
}

std::vector<Bitboard> subsets(Bitboard mask)
{
    std::vector<Bitboard> out {};

    Bitboard subset = Bitboard{};

    do {
        out.emplace_back(subset);
        subset = Bitboard((std::uint64_t(subset) - std::uint64_t(mask)) & std::uint64_t(mask));
    } while (!subset.is_empty());

    return out;
}

template <Slider SLIDER>
std::optional<std::vector<Bitboard>> try_make_table(Square square, MagicEntry magicEntry)
{
    std::vector<Bitboard> table;
    for (int i = 0; i < 1 << magicEntry.indexBits; ++i) {
        table.emplace_back();
    }

    for (Bitboard blockers : subsets(magicEntry.mask)) {
        Bitboard  moves      = generate_slider_moves<SLIDER>(square, blockers);
        Bitboard& tableEntry = table[magic_index(magicEntry, blockers)];
        if (tableEntry == Bitboard()) {
            tableEntry = moves;
        }
        else if (tableEntry != moves) {
            return std::nullopt;
        }
    }

    return table;
}

template <Slider SLIDER>
std::pair<MagicEntry, std::vector<Bitboard>> find_magic(Square square, std::uint8_t indexBits)
{
    const Bitboard  mask = relevant_blockers<SLIDER>(square);
    std::mt19937_64 rng{};
    while (true) {
        std::uint64_t         magic = rng() & rng() & rng();
        MagicEntry            magicEntry{mask, magic, indexBits};
        std::optional<std::vector<Bitboard>> t;
        if ((t = try_make_table<SLIDER>(square, magicEntry))) {
            return {magicEntry, t.value()};
        }
    }
}
} // namespace

void init_magics()
{
    // Standard index bits for Rooks and Bishops
    static constexpr std::uint8_t ROOK_BITS[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12
    };

    static constexpr std::uint8_t BISHOP_BITS[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6
    };

    for (int i = 0; i < 64; ++i) {
        Square sq = static_cast<Square>(i);

        auto [rEntry, rTable] = find_magic<Slider::ROOK>(sq, ROOK_BITS[i]);
        rookMagics[i] = rEntry;
        rookMoves[i]  = std::move(rTable);

        auto [bEntry, bTable] = find_magic<Slider::BISHOP>(sq, BISHOP_BITS[i]);
        bishopMagics[i] = bEntry;
        bishopMoves[i]  = std::move(bTable);
    }
}

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
    return get_bishop_moves(src, blockers);
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::ROOK>, Bitboard>
generate_attacks<PieceType::ROOK>(const Square src, const Bitboard blockers)
{
    return get_rook_moves(src, blockers);
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
