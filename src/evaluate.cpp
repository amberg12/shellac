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

// Piece-Square tables taken from Simplified Evaluation Function (CPW)

#include "evaluate.h"
#include "position.h"
#include "types.h"

namespace shellac {
namespace {
using Psqt = std::array<Score, 64>;

Score evaluate_pawns(const Position& position)
{
    // clang-format off
    constexpr Psqt PAWN_TABLE = {
        0,   0,   0,   0,   0,   0,   0,   0,
        5,  10,  10, -20, -20,  10,  10,   5,
        5,  -5, -10,   0,   0, -10,  -5,   5,
        0,   0,   0,  20,  20,   0,   0,   0,
        5,   5,  10,  25,  25,  10,   5,   5,
       10,  10,  20,  30,  30,  20,  10,  10,
       50,  50,  50,  50,  50,  50,  50,  50,
        0,   0,   0,   0,   0,   0,   0,   0
    };
    //clang-format on
    Score ret = 0;
    for (const Square square : position.pieces(Color::WHITE, PieceType::PAWN)) {
        ret += PAWN_SCORE;
        ret += PAWN_TABLE[underlying(square)];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::PAWN)) {
        ret -= PAWN_SCORE;
        ret -= PAWN_TABLE[underlying(mirror(square))];
    }

    return ret;
}

Score evaluate_knights(const Position& position)
{
    // clang-format off
    constexpr Psqt KNIGHT_TABLE = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
     };
    // clang-format on

    Score ret = 0;

    for (const Square square : position.pieces(Color::WHITE, PieceType::KNIGHT)) {
        ret += KNIGHT_SCORE;
        ret += KNIGHT_TABLE[underlying(square)];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::KNIGHT)) {
        ret -= KNIGHT_SCORE;
        ret -= KNIGHT_TABLE[underlying(mirror(square))];
    }

    return ret;
}

Score evaluate_bishops(const Position& position)
{
    // clang-format off
    constexpr Psqt BISHOP_TABLE = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
     };
    // clang-format on

    Score ret = 0;

    for (const Square square : position.pieces(Color::WHITE, PieceType::BISHOP)) {
        ret += BISHOP_SCORE;
        ret += BISHOP_TABLE[underlying(square)];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::BISHOP)) {
        ret -= BISHOP_SCORE;
        ret -= BISHOP_TABLE[underlying(mirror(square))];
    }

    return ret;
}

Score evaluate_rooks(const Position& position)
{
    // clang-format off
    constexpr Psqt ROOK_TABLE = {
        0,   0,   0,   5,   5,   0,   0,   0,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
       -5,   0,   0,   0,   0,   0,   0,  -5,
        5,  10,  10,  10,  10,  10,  10,   5,
        0,   0,   0,   0,   0,   0,   0,   0
   };
    // clang-format on

    Score ret = 0;

    for (const Square square : position.pieces(Color::WHITE, PieceType::ROOK)) {
        ret += ROOK_SCORE;
        ret += ROOK_TABLE[underlying(square)];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::ROOK)) {
        ret -= ROOK_SCORE;
        ret -= ROOK_TABLE[underlying(mirror(square))];
    }

    return ret;
}

Score evaluate_queens(const Position& position)
{
    // clang-format off
    constexpr Psqt QUEEN_TABLE = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   5,   0,   0,   5,   0, -10,
        -10,   5,   5,   5,   5,   5,   5, -10,
          0,   0,   5,   5,   5,   5,   0,  -5,
         -5,   0,   5,   5,   5,   5,   0,  -5,
        -10,   0,   5,   5,   5,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
     };
    // clang-format on

    Score ret = 0;

    for (const Square square : position.pieces(Color::WHITE, PieceType::QUEEN)) {
        ret += QUEEN_SCORE;
        ret += QUEEN_TABLE[underlying(square)];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::QUEEN)) {
        ret -= QUEEN_SCORE;
        ret -= QUEEN_TABLE[underlying(mirror(square))];
    }

    return ret;
}

Score evaluate_kings(const Position& position)
{
    constexpr Score KING_TABLE[64] = {
        20,  30,  10,   0,   0,  10,  30,  20,
        20,  20,   0,   0,   0,   0,  20,  20,
       -10, -20, -20, -20, -20, -20, -20, -10,
       -20, -30, -30, -40, -40, -30, -30, -20,
       -30, -40, -40, -50, -50, -40, -40, -30,
       -30, -40, -40, -50, -50, -40, -40, -30,
       -30, -40, -40, -50, -50, -40, -40, -30,
       -30, -40, -40, -50, -50, -40, -40, -30
    };
    // clang-format on
    Score ret = 0;
    ret += KING_TABLE[underlying(position.king_square(Color::WHITE))];
    ret -= KING_TABLE[underlying(mirror(position.king_square(Color::BLACK)))];
    return ret;
}
}

Score evaluate(const Position& position)
{
    Score score = 0;

    score += evaluate_pawns(position);
    score += evaluate_knights(position);
    score += evaluate_bishops(position);
    score += evaluate_rooks(position);
    score += evaluate_queens(position);
    score += evaluate_kings(position);

    return (position.side_to_move() == Color::WHITE) ? score : -score;}
} // namespace shellac
