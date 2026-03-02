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

//
// Created by amber on 02/03/2026.
//

#ifndef SHELLAC_TYPES_H
#define SHELLAC_TYPES_H
#include <cassert>
#include <cstdint>

#include "defs.h"

namespace shellac {

constexpr const char* STARTING_POSITION =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum class Color : std::uint8_t
{
    WHITE = 0,
    BLACK,
    NB = 2,
};

template <>
constexpr Color from_char<Color>(const char c)
{
    return c == 'w' ? Color::WHITE : Color::BLACK;
}

enum class PieceType : std::uint8_t
{
    NONE = 0,
    PAWN = 1,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
};

enum class Piece : std::uint8_t
{
    NONE   = 0,
    W_PAWN = static_cast<std::uint8_t>(PieceType::PAWN),
    W_KNIGHT,
    W_BISHOP,
    W_ROOK,
    W_QUEEN,
    W_KING,
    B_PAWN = W_PAWN + 8,
    B_KNIGHT,
    B_BISHOP,
    B_ROOK,
    B_QUEEN,
    B_KING,
};

template <>
constexpr Piece from_char<Piece>(const char c)
{
    switch (c) {
    case 'P':
        return Piece::W_PAWN;
    case 'N':
        return Piece::W_KNIGHT;
    case 'B':
        return Piece::W_BISHOP;
    case 'R':
        return Piece::W_ROOK;
    case 'Q':
        return Piece::W_QUEEN;
    case 'K':
        return Piece::W_KING;
    case 'p':
        return Piece::B_PAWN;
    case 'n':
        return Piece::B_KNIGHT;
    case 'b':
        return Piece::B_BISHOP;
    case 'r':
        return Piece::B_ROOK;
    case 'q':
        return Piece::B_QUEEN;
    case 'k':
        return Piece::B_KING;
    default:
        return Piece::NONE;
    }
}

constexpr char to_char(const Piece piece)
{
    switch (piece) {
    case Piece::W_PAWN:
        return 'P';
    case Piece::W_KNIGHT:
        return 'N';
    case Piece::W_BISHOP:
        return 'B';
    case Piece::W_ROOK:
        return 'R';
    case Piece::W_QUEEN:
        return 'Q';
    case Piece::W_KING:
        return 'K';
    case Piece::B_PAWN:
        return 'p';
    case Piece::B_KNIGHT:
        return 'n';
    case Piece::B_BISHOP:
        return 'b';
    case Piece::B_ROOK:
        return 'r';
    case Piece::B_QUEEN:
        return 'q';
    case Piece::B_KING:
        return 'k';
    default:
        return ' ';
    }
}

enum class File : std::uint8_t
{
    F_A,
    F_B,
    F_C,
    F_D,
    F_E,
    F_F,
    F_G,
    F_H,
    INVALID,
    NB = 8,
};

constexpr File& operator++(File& file)
{
    file = static_cast<File>(underlying(file) + 1);
    return file;
}

constexpr File& operator+=(File& file, int rhs)
{
    file = static_cast<File>(underlying(file) + rhs);
    return file;
}

constexpr bool is_valid(const File file)
{
    return File::F_A <= file && file <= File::F_H;
}

template <>
constexpr File from_char(const char c)
{
    const File file = static_cast<File>(c - std::isupper(c) ? 'A' : 'a');
    assert(is_valid(file));
    return file;
}

constexpr char to_char(const File file)
{
    assert(is_valid(file));
    return static_cast<char>('a' + underlying(file));
}

enum class Rank : std::uint8_t
{
    R_1 = 0,
    R_2,
    R_3,
    R_4,
    R_5,
    R_6,
    R_7,
    R_8,
    INVALID,
    NB = 8,
};

constexpr Rank& operator--(Rank& rank)
{
    rank = static_cast<Rank>(underlying(rank) - 1);
    return rank;
}

constexpr bool is_valid(const Rank rank)
{
    return Rank::R_1 <= rank && rank <= Rank::R_8;
}

template <>
constexpr Rank from_char<Rank>(const char c)
{
    const Rank rank = static_cast<Rank>(c - '0');
    assert(is_valid(rank));
    return rank;
}

constexpr char to_char(const Rank rank)
{
    assert(is_valid(rank));
    return static_cast<char>('1' + underlying(rank));
}

enum class Square : std::uint8_t
{
    A1 = 0,
    B1,
    C1,
    D1,
    E1,
    F1,
    G1,
    H1,
    A2,
    B2,
    C2,
    D2,
    E2,
    F2,
    G2,
    H2,
    A3,
    B3,
    C3,
    D3,
    E3,
    F3,
    G3,
    H3,
    A4,
    B4,
    C4,
    D4,
    E4,
    F4,
    G4,
    H4,
    A5,
    B5,
    C5,
    D5,
    E5,
    F5,
    G5,
    H5,
    A6,
    B6,
    C6,
    D6,
    E6,
    F6,
    G6,
    H6,
    A7,
    B7,
    C7,
    D7,
    E7,
    F7,
    G7,
    H7,
    A8,
    B8,
    C8,
    D8,
    E8,
    F8,
    G8,
    H8,
    NB      = 64,
    INVALID = 64,
};

constexpr bool is_valid(const Square square)
{
    return Square::A1 <= square && square <= Square::H8;
}

constexpr Square make_square(const File file, const Rank rank)
{
    assert(is_valid(file));
    assert(is_valid(rank));
    const std::uint8_t square_value = underlying(file) + underlying(rank) * 8;
    return static_cast<Square>(square_value);
}

template <>
constexpr Square from_string<Square>(const std::string_view s)
{
    File file = File::INVALID;
    Rank rank = Rank::INVALID;
    if (s.size() == 2) {
        file = from_char<File>(s[0]);
        rank = from_char<Rank>(s[1]);
    }

    if (is_valid(file) && is_valid(rank)) {
        return make_square(file, rank);
    }

    return Square::INVALID;
}

constexpr File file_of(const Square square)
{
    assert(is_valid(square));
    return static_cast<File>(underlying(square) & 7);
}

constexpr Rank rank_of(const Square square)
{
    assert(is_valid(square));
    return static_cast<Rank>(underlying(square) >> 3);
}

} // namespace shellac

#endif // SHELLAC_TYPES_H
