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

namespace shellac {
namespace {
Score evaluate_pawns(const Position& position)
{
    const Score whitePawns = position.pieces(Color::WHITE, PieceType::PAWN).pop_count() * Material::PAWN_SCORE;
    const Score blackPawns = position.pieces(Color::BLACK, PieceType::PAWN).pop_count() * Material::PAWN_SCORE;
    return whitePawns - blackPawns;
}

Score evaluate_knights(const Position& position)
{
    const Score whiteKnights =
        position.pieces(Color::WHITE, PieceType::KNIGHT).pop_count() * Material::KNIGHT_SCORE;
    const Score blackKnights =
        position.pieces(Color::BLACK, PieceType::KNIGHT).pop_count() * Material::KNIGHT_SCORE;
    return whiteKnights - blackKnights;
}

Score evaluate_bishops(const Position& position)
{
    const Score whiteBishops =
        position.pieces(Color::WHITE, PieceType::BISHOP).pop_count() * Material::BISHOP_SCORE;
    const Score blackBishops =
        position.pieces(Color::BLACK, PieceType::BISHOP).pop_count() * Material::BISHOP_SCORE;
    return whiteBishops - blackBishops;
}

Score evaluate_rooks(const Position& position)
{
    const Score whiteRooks =
        position.pieces(Color::WHITE, PieceType::ROOK).pop_count() * Material::ROOK_SCORE;
    const Score blackRooks =
        position.pieces(Color::BLACK, PieceType::ROOK).pop_count() * Material::ROOK_SCORE;
    return whiteRooks - blackRooks;
}

Score evaluate_queens(const Position& position)
{
    const Score whiteQueens =
        position.pieces(Color::WHITE, PieceType::QUEEN).pop_count() * Material::QUEEN_SCORE;
    const Score blackQueens =
        position.pieces(Color::BLACK, PieceType::QUEEN).pop_count() * Material::QUEEN_SCORE;
    return whiteQueens - blackQueens;
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

    return position.side_to_move() == Color::WHITE ? score : -score;
}
} // namespace shellac
