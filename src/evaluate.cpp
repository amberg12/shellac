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

#include "evaluate.h"
#include "position.h"
#include "types.h"

// All values are taken from the PeSTO evaluation function.
// Before updating the eval, these should be tuned.

namespace shellac {
namespace {

enum class Phase
{
    PAWN   = 0,
    KNIGHT = 1,
    BISHOP = 1,
    ROOK   = 2,
    QUEEN  = 4,
    KING   = 0,
    TOTAL  = 24,
};

int calculate_phase(const Position& pos)
{
    int phase = 0;
    phase += underlying(Phase::PAWN) * pos.pieces(PieceType::PAWN).pop_count();
    phase += underlying(Phase::KNIGHT) * pos.pieces(PieceType::KNIGHT).pop_count();
    phase += underlying(Phase::BISHOP) * pos.pieces(PieceType::BISHOP).pop_count();
    phase += underlying(Phase::ROOK) * pos.pieces(PieceType::ROOK).pop_count();
    phase += underlying(Phase::QUEEN) * pos.pieces(PieceType::QUEEN).pop_count();
    return phase;
}

using Psqt = std::array<Score, 64>;

struct EvalResult
{
    Score mg{};
    Score eg{};
};

EvalResult evaluate_pawns(const Position& position)
{
    constexpr Psqt MG_PAWN_TABLE = {
          0,   0,   0,   0,   0,   0,  0,   0,
         98, 134,  61,  95,  68, 126, 34, -11,
         -6,   7,  26,  31,  65,  56, 25, -20,
        -14,  13,   6,  21,  23,  12, 17, -23,
        -27,  -2,  -5,  12,  17,   6, 10, -25,
        -26,  -4,  -4, -10,   3,   3, 33, -12,
        -35,  -1, -20, -23, -15,  24, 38, -22,
          0,   0,   0,   0,   0,   0,  0,   0,
    };

    constexpr Psqt EG_PAWN_TABLE = {
          0,   0,   0,   0,   0,   0,   0,   0,
        178, 173, 158, 134, 147, 132, 165, 187,
         94, 100,  85,  67,  56,  53,  82,  84,
         32,  24,  13,   5,  -2,   4,  17,  17,
         13,   9,  -3,  -7,  -7,  -8,   3,  -1,
          4,   7,  -6,   1,   0,  -5,  -1,  -8,
         13,   8,   8,  10,  13,   0,   2,  -7,
          0,   0,   0,   0,   0,   0,   0,   0,
    };

    constexpr Score MG_PAWN_VALUE = 82;
    constexpr Score EG_PAWN_VALUE = 94;

    EvalResult ret{};

    for (const Square square : position.pieces(Color::WHITE, PieceType::PAWN)) {
        ret.mg += MG_PAWN_VALUE + MG_PAWN_TABLE[underlying(mirror(square))];
        ret.eg += EG_PAWN_VALUE + EG_PAWN_TABLE[underlying(mirror(square))];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::PAWN)) {
        ret.mg -= MG_PAWN_VALUE + MG_PAWN_TABLE[underlying(square)];
        ret.eg -= EG_PAWN_VALUE + EG_PAWN_TABLE[underlying(square)];
    }

    return ret;
}

EvalResult evaluate_knights(const Position& position)
{
    constexpr Psqt MG_KNIGHT_TABLE = {
        -167, -89, -34, -49,  61, -97, -15, -107,
         -73, -41,  72,  36,  23,  62,   7,  -17,
         -47,  60,  37,  65,  84, 129,  73,   44,
          -9,  17,  19,  53,  37,  69,  18,   22,
         -13,   4,  16,  13,  28,  19,  21,   -8,
         -23,  -9,  12,  10,  19,  17,  25,  -16,
         -29, -53, -12,  -3,  -1,  18, -14,  -19,
        -105, -21, -58, -33, -17, -28, -19,  -23,
    };

    constexpr Psqt EG_KNIGHT_TABLE = {
        -58, -38, -13, -28, -31, -27, -63, -99,
        -25,  -8, -25,  -2,  -9, -25, -24, -52,
        -24, -20,  10,   9,  -1,  -9, -19, -41,
        -17,   3,  22,  22,  22,  11,   8, -18,
        -18,  -6,  16,  25,  16,  17,   4, -18,
        -23,  -3,  -1,  15,  10,  -3, -20, -22,
        -42, -20, -10,  -5,  -2, -20, -23, -44,
        -29, -51, -23, -15, -22, -18, -50, -64,
    };

    constexpr Score MG_KNIGHT_VALUE = 337;
    constexpr Score EG_KNIGHT_VALUE = 281;

    EvalResult ret{};

    for (const Square square : position.pieces(Color::WHITE, PieceType::KNIGHT)) {
        ret.mg += MG_KNIGHT_VALUE + MG_KNIGHT_TABLE[underlying(mirror(square))];
        ret.eg += EG_KNIGHT_VALUE + EG_KNIGHT_TABLE[underlying(mirror(square))];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::KNIGHT)) {
        ret.mg -= MG_KNIGHT_VALUE + MG_KNIGHT_TABLE[underlying(square)];
        ret.eg -= EG_KNIGHT_VALUE + EG_KNIGHT_TABLE[underlying(square)];
    }

    return ret;
}

EvalResult evaluate_bishops(const Position& position)
{
    constexpr Psqt MG_BISHOP_TABLE = {
        -29,   4, -82, -37, -25, -42,   7,  -8,
        -26,  16, -18, -13,  30,  59,  18, -47,
        -16,  37,  43,  40,  35,  50,  37,  -2,
         -4,   5,  19,  50,  37,  37,   7,  -2,
         -6,  13,  13,  26,  34,  12,  10,   4,
          0,  15,  15,  15,  14,  27,  18,  10,
          4,  15,  16,   0,   7,  21,  33,   1,
        -33,  -3, -14, -21, -13, -12, -39, -21,
    };

    constexpr Psqt EG_BISHOP_TABLE = {
        -14, -21, -11,  -8, -7,  -9, -17, -24,
         -8,  -4,   7, -12, -3, -13,  -4, -14,
          2,  -8,   0,  -1, -2,   6,   0,   4,
         -3,   9,  12,   9, 14,  10,   3,   2,
         -6,   3,  13,  19,  7,  10,  -3,  -9,
        -12,  -3,   8,  10, 13,   3,  -7, -15,
        -14, -18,  -7,  -1,  4,  -9, -15, -27,
        -23,  -9, -23,  -5, -9, -16,  -5, -17,
    };

    constexpr Score MG_BISHOP_VALUE = 365;
    constexpr Score EG_BISHOP_VALUE = 297;

    EvalResult ret{};

    for (const Square square : position.pieces(Color::WHITE, PieceType::BISHOP)) {
        ret.mg += MG_BISHOP_VALUE + MG_BISHOP_TABLE[underlying(mirror(square))];
        ret.eg += EG_BISHOP_VALUE + EG_BISHOP_TABLE[underlying(mirror(square))];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::BISHOP)) {
        ret.mg -= MG_BISHOP_VALUE + MG_BISHOP_TABLE[underlying(square)];
        ret.eg -= EG_BISHOP_VALUE + EG_BISHOP_TABLE[underlying(square)];
    }

    return ret;
}

EvalResult evaluate_rooks(const Position& position)
{
    constexpr Psqt MG_ROOK_TABLE = {
         32,  42,  32,  51, 63,  9,  31,  43,
         27,  32,  58,  62, 80, 67,  26,  44,
         -5,  19,  26,  36, 17, 45,  61,  16,
        -24, -11,   7,  26, 24, 35,  -8, -20,
        -36, -26, -12,  -1,  9, -7,   6, -23,
        -45, -25, -16, -17,  3,  0,  -5, -33,
        -44, -16, -20,  -9, -1, 11,  -6, -71,
        -19, -13,   1,  17, 16,  7, -37, -26,
    };

    constexpr Psqt EG_ROOK_TABLE = {
         13, 10, 18, 15, 12,  12,   8,   5,
         11, 13, 13, 11, -3,   3,   8,   3,
          7,  7,  7,  5,  4,  -3,  -5,  -3,
          4,  3, 13,  1,  2,   1,  -1,   2,
          3,  5,  8,  4, -5,  -6,  -8, -11,
         -4,  0, -5, -1, -7, -12,  -8, -16,
         -6, -6,  0,  2, -9,  -9, -11,  -3,
         -9,  2,  3, -1, -5, -13,   4, -20,
    };

    constexpr Score MG_ROOK_VALUE = 477;
    constexpr Score EG_ROOK_VALUE = 512;

    EvalResult ret{};

    for (const Square square : position.pieces(Color::WHITE, PieceType::ROOK)) {
        ret.mg += MG_ROOK_VALUE + MG_ROOK_TABLE[underlying(mirror(square))];
        ret.eg += EG_ROOK_VALUE + EG_ROOK_TABLE[underlying(mirror(square))];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::ROOK)) {
        ret.mg -= MG_ROOK_VALUE + MG_ROOK_TABLE[underlying(square)];
        ret.eg -= EG_ROOK_VALUE + EG_ROOK_TABLE[underlying(square)];
    }

    return ret;
}

EvalResult evaluate_queens(const Position& position)
{
    constexpr Psqt MG_QUEEN_TABLE = {
        -28,   0,  29,  12,  59,  44,  43,  45,
        -24, -39,  -5,   1, -16,  57,  28,  54,
        -13, -17,   7,   8,  29,  56,  47,  57,
        -27, -27, -16, -16,  -1,  17,  -2,   1,
         -9, -26,  -9, -10,  -2,  -4,   3,  -3,
        -14,   2, -11,  -2,  -5,   2,  14,   5,
        -35,  -8,  11,   2,   8,  15,  -3,   1,
         -1, -18,  -9,  10, -15, -25, -31, -50,
    };

    constexpr Psqt EG_QUEEN_TABLE = {
         -9,  22,  22,  27,  27,  19,  10,  20,
        -17,  20,  32,  41,  58,  25,  30,   0,
        -20,   6,   9,  49,  47,  35,  19,   9,
          3,  22,  24,  45,  57,  40,  57,  36,
        -18,  28,  19,  47,  31,  34,  39,  23,
        -16, -27,  15,   6,   9,  17,  10,   5,
        -22, -23, -30, -16, -16, -23, -36, -32,
        -33, -28, -22, -43,  -5, -32, -20, -41,
    };

    constexpr Score MG_QUEEN_VALUE = 1025;
    constexpr Score EG_QUEEN_VALUE = 936;

    EvalResult ret{};

    for (const Square square : position.pieces(Color::WHITE, PieceType::QUEEN)) {
        ret.mg += MG_QUEEN_VALUE + MG_QUEEN_TABLE[underlying(mirror(square))];
        ret.eg += EG_QUEEN_VALUE + EG_QUEEN_TABLE[underlying(mirror(square))];
    }

    for (const Square square : position.pieces(Color::BLACK, PieceType::QUEEN)) {
        ret.mg -= MG_QUEEN_VALUE + MG_QUEEN_TABLE[underlying(square)];
        ret.eg -= EG_QUEEN_VALUE + EG_QUEEN_TABLE[underlying(square)];
    }

    return ret;
}

EvalResult evaluate_kings(const Position& position)
{
    constexpr Psqt MG_KING_TABLE = {
        -65,  23,  16, -15, -56, -34,   2,  13,
         29,  -1, -20,  -7,  -8,  -4, -38, -29,
         -9,  24,   2, -16, -20,   6,  22, -22,
        -17, -20, -12, -27, -30, -25, -14, -36,
        -49,  -1, -27, -39, -46, -44, -33, -51,
        -14, -14, -22, -46, -44, -30, -15, -27,
          1,   7,  -8, -64, -43, -16,   9,   8,
        -15,  36,  12, -54,   8, -28,  24,  14,
};

    constexpr Psqt EG_KING_TABLE = {
        -74, -35, -18, -18, -11,  15,   4, -17,
        -12,  17,  14,  17,  17,  38,  23,  11,
         10,  17,  23,  15,  20,  45,  44,  13,
         -8,  22,  24,  27,  26,  33,  26,   3,
        -18,  -4,  21,  24,  27,  23,   9, -11,
        -19,  -3,  11,  21,  23,  16,   7,  -9,
        -27, -11,   4,  13,  14,   4,  -5, -17,
        -53, -34, -21, -11, -28, -14, -24, -43,
    };

    EvalResult ret{};

    ret.mg += MG_KING_TABLE[underlying(mirror(position.king_square(Color::WHITE)))];
    ret.eg += EG_KING_TABLE[underlying(mirror(position.king_square(Color::WHITE)))];
    ret.mg -= MG_KING_TABLE[underlying(position.king_square(Color::BLACK))];
    ret.eg -= EG_KING_TABLE[underlying(position.king_square(Color::BLACK))];

    return ret;
}

} // namespace

Score evaluate(const Position& position)
{
    const int phase   = calculate_phase(position);
    const int mgPhase = (phase > 24) ? 24 : phase;
    const int egPhase = 24 - mgPhase;

    EvalResult pawns   = evaluate_pawns(position);
    EvalResult knights = evaluate_knights(position);
    EvalResult bishops = evaluate_bishops(position);
    EvalResult rooks   = evaluate_rooks(position);
    EvalResult queens  = evaluate_queens(position);
    EvalResult kings   = evaluate_kings(position);

    const Score mgScore = pawns.mg + knights.mg + bishops.mg + rooks.mg + queens.mg + kings.mg;
    const Score egScore = pawns.eg + knights.eg + bishops.eg + rooks.eg + queens.eg + kings.eg;

    const Score score = (mgScore * mgPhase + egScore * egPhase) / 24;

    return (position.side_to_move() == Color::WHITE) ? score : -score;
}

} // namespace shellac