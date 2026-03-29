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
#include <iostream>

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

constexpr Color operator~(const Color c)
{
    assert(c == Color::WHITE || c == Color::BLACK);
    return static_cast<Color>(underlying(c) ^ 1);
}

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

template <PieceType PIECE_TYPE>
struct is_pawn
{
    static constexpr bool value = PIECE_TYPE == PieceType::PAWN;
};

template <PieceType PIECE_TYPE>
constexpr bool is_pawn_v = is_pawn<PIECE_TYPE>::value;

template <PieceType PIECE_TYPE>
struct is_knight
{
    static constexpr bool value = PIECE_TYPE == PieceType::KNIGHT;
};

template <PieceType PIECE_TYPE>
constexpr bool is_knight_v = is_knight<PIECE_TYPE>::value;

template <PieceType PIECE_TYPE>
struct is_slider
{
    static constexpr bool value = PIECE_TYPE == PieceType::BISHOP ||
        PIECE_TYPE == PieceType::ROOK || PIECE_TYPE == PieceType::QUEEN;
};

template <PieceType PIECE_TYPE>
constexpr bool is_slider_v = is_slider<PIECE_TYPE>::value;

template <PieceType PIECE_TYPE>
struct is_king
{
    static constexpr bool value = PIECE_TYPE == PieceType::KING;
};

template <PieceType PIECE_TYPE>
constexpr bool is_king_v = is_king<PIECE_TYPE>::value;

constexpr char to_char(const PieceType piece)
{
    switch (piece) {
    case PieceType::PAWN:
        return 'p';
    case PieceType::KNIGHT:
        return 'n';
    case PieceType::BISHOP:
        return 'b';
    case PieceType::ROOK:
        return 'r';
    case PieceType::QUEEN:
        return 'q';
    case PieceType::KING:
        return 'k';
    default:
        return '_';
    }
}

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

constexpr PieceType type_of(const Piece piece)
{
    assert((Piece::W_PAWN <= piece && piece <= Piece::W_KING) ||
           (Piece::B_PAWN <= piece && piece <= Piece::B_KING) || piece == Piece::NONE);
    return static_cast<PieceType>(underlying(piece) & 0b111);
}

constexpr Color color_of(const Piece piece)
{
    assert((Piece::W_PAWN <= piece && piece <= Piece::W_KING) ||
           (Piece::B_PAWN <= piece && piece <= Piece::B_KING) || piece == Piece::NONE);
    return underlying(piece) & 0b1000 ? Color::BLACK : Color::WHITE;
}

constexpr Piece make_piece(const Color color, const PieceType pieceType)
{
    if (color == Color::WHITE) {
        return static_cast<Piece>(underlying(pieceType));
    }
    else {
        return static_cast<Piece>(0b1000 | underlying(pieceType));
    }
}

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

constexpr File operator+(const File file, const int diff)
{
    return static_cast<File>(underlying(file) + diff);
}

constexpr File& operator++(File& file)
{
    file = static_cast<File>(underlying(file) + 1);
    return file;
}

constexpr File& operator+=(File& file, const int rhs)
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
    const File file = static_cast<File>(c - (std::isupper(c) ? 'A' : 'a'));
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

constexpr Rank operator+(const Rank rank, const int diff)
{
    return static_cast<Rank>(underlying(rank) + diff);
}

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
    const Rank rank = static_cast<Rank>(c - '1');
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

constexpr Square mirror(const Square square)
{
    return static_cast<Square>(underlying(square) ^ 56);
}

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

enum class Direction
{
    NORTH = 8,
    SOUTH = -8,
    EAST  = 1,
    WEST  = -1,
    NONE  = 0,
};

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

constexpr Square& operator+=(Square& lhs, const Direction rhs)
{
    const File originalFile = file_of(lhs);
    const Rank originalRank = rank_of(lhs);

    assert(!(originalFile == File::F_A && rhs == Direction::WEST));
    assert(!(originalFile == File::F_H && rhs == Direction::EAST));
    assert(!(originalRank == Rank::R_1 && rhs == Direction::SOUTH));
    assert(!(originalRank == Rank::R_8 && rhs == Direction::NORTH));

    lhs = static_cast<Square>(underlying(lhs) + underlying(rhs));
    return lhs;
}

constexpr Square operator+(const Square lhs, const Direction rhs)
{
    const File originalFile = file_of(lhs);
    const Rank originalRank = rank_of(lhs);

    assert(!(originalFile == File::F_A && rhs == Direction::WEST));
    assert(!(originalFile == File::F_H && rhs == Direction::EAST));
    assert(!(originalRank == Rank::R_1 && rhs == Direction::SOUTH));
    assert(!(originalRank == Rank::R_8 && rhs == Direction::NORTH));

    return static_cast<Square>(underlying(lhs) + underlying(rhs));
}

constexpr bool is_orthogonal_to(const Square lhs, const Square rhs)
{
    assert(is_valid(lhs) && is_valid(rhs));
    return file_of(lhs) == file_of(rhs) || rank_of(lhs) == rank_of(rhs);
}

constexpr bool is_diagonal_to(const Square lhs, const Square rhs)
{
    assert(is_valid(lhs) && is_valid(rhs));

    const int df =
        static_cast<int>(underlying(file_of(lhs))) - static_cast<int>(underlying(file_of(rhs)));
    const int dr =
        static_cast<int>(underlying(rank_of(lhs))) - static_cast<int>(underlying(rank_of(rhs)));

    return std::abs(df) == std::abs(dr);
}

inline std::string to_string(const Square square)
{
    std::string out;
    out += to_char(file_of(square));
    out += to_char(rank_of(square));
    return out;
}

class Move
{
public:
    Move() = default;
    explicit Move(const std::uint16_t repr) : repr_(repr)
    {
    }
    Move(const Square src, const Square dst) : repr_((underlying(dst) << 6) + underlying(src))
    {
    }

    static Move create_en_passant(const Square src, const Square dst)
    {
        Move out{src, dst};
        out.repr_ |= EN_PASSANT;
        return out;
    }

    static Move create_castle(const Square src, const Square dst)
    {
        Move out{src, dst};
        out.repr_ |= CASTLING;
        return out;
    }

    static Move create_promotion(const Square src, const Square dst, const PieceType to)
    {
        assert(to == PieceType::KNIGHT || to == PieceType::BISHOP || to == PieceType::ROOK ||
               to == PieceType::QUEEN);
        Move out{src, dst};
        out.repr_ |= PROMOTION;
        out.repr_ |= (underlying(to) - underlying(PieceType::KNIGHT)) << 12;
        return out;
    }

    [[nodiscard]] Square src() const
    {
        return static_cast<Square>(repr_ & SRC);
    }

    [[nodiscard]] Square dst() const
    {
        return static_cast<Square>((repr_ & DST) >> 6);
    }

    [[nodiscard]] bool is_en_passant() const
    {
        return (repr_ & SPECIAL) == EN_PASSANT;
    }

    [[nodiscard]] bool is_castle() const
    {
        return (repr_ & SPECIAL) == CASTLING;
    }

    [[nodiscard]] bool is_promotion() const
    {
        return (repr_ & PROMOTION) == PROMOTION;
    }

    [[nodiscard]] bool is_null() const
    {
        return repr_ == 0;
    }

    [[nodiscard]] PieceType promotion_piece() const
    {
        assert(is_promotion());
        return static_cast<PieceType>(((repr_ & PROMOTE_TO) >> 12) + underlying(PieceType::KNIGHT));
    }

    [[nodiscard]] bool operator==(const Move rhs) const
    {
        return repr_ == rhs.repr_;
    }

    [[nodiscard]] bool operator!=(const Move rhs) const
    {
        return repr_ != rhs.repr_;
    }

private:
    enum Masks : std::uint16_t
    {
        SRC        = 0b0000'0000'0011'1111,
        DST        = 0b0000'1111'1100'0000,
        SPECIAL    = 0b1111'0000'0000'0000,
        EN_PASSANT = 0b0001'0000'0000'0000,
        CASTLING   = 0b0010'0000'0000'0000,
        PROMOTION  = 0b0100'0000'0000'0000,
        PROMOTE_TO = 0b0011'0000'0000'0000,
    };

    std::uint16_t repr_{0};
};

inline std::string to_string(const Move move)
{
    if (move.is_null()) {
        return "0000";
    }

    std::string out = to_string(move.src()) + to_string(move.dst());
    if (move.is_promotion()) {
        out += to_char(move.promotion_piece());
    }

    return out;
}

inline std::ostream& operator<<(std::ostream& os, const Move& move)
{
    os << to_string(move);
    return os;
}

using Score                = std::int16_t;
constexpr Score MATE_SCORE = -30'000;
constexpr Score DRAW_SCORE = 0;
constexpr Score NEG_INF    = -31'000;
constexpr Score POS_INF    = 31'000;
constexpr Score NO_SCORE   = -32'000;

enum Material : Score
{
    PAWN_SCORE   = 100,
    KNIGHT_SCORE = 320,
    BISHOP_SCORE = 330,
    ROOK_SCORE   = 500,
    QUEEN_SCORE  = 900,
};

constexpr Score evaluate_piece(const PieceType piece)
{
    switch (piece) {
    case PieceType::PAWN:
        return PAWN_SCORE;
    case PieceType::KNIGHT:
        return KNIGHT_SCORE;
    case PieceType::BISHOP:
        return BISHOP_SCORE;
    case PieceType::ROOK:
        return ROOK_SCORE;
    case PieceType::QUEEN:
        return QUEEN_SCORE;
    default:
        return 0;
    }
}

enum class Bounds
{
    UPPER,
    LOWER,
    EXACT,
};

} // namespace shellac

#endif // SHELLAC_TYPES_H
