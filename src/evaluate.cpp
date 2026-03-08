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
    const Score whitePawns = position.pieces(Color::WHITE, PieceType::PAWN).pop_count() * Material::PAWN;
    const Score blackPawns = position.pieces(Color::BLACK, PieceType::PAWN).pop_count() * Material::PAWN;
    return whitePawns - blackPawns;
}

Score evaluate_knights(const Position& position)
{
    const Score whiteKnights =
        position.pieces(Color::WHITE, PieceType::KNIGHT).pop_count() * Material::KNIGHT;
    const Score blackKnights =
        position.pieces(Color::BLACK, PieceType::KNIGHT).pop_count() * Material::KNIGHT;
    return whiteKnights - blackKnights;
}

Score evaluate_bishops(const Position& position)
{
    const Score whiteBishops =
        position.pieces(Color::WHITE, PieceType::BISHOP).pop_count() * Material::BISHOP;
    const Score blackBishops =
        position.pieces(Color::BLACK, PieceType::BISHOP).pop_count() * Material::BISHOP;
    return whiteBishops - blackBishops;
}

Score evaluate_rooks(const Position& position)
{
    const Score whiteRooks =
        position.pieces(Color::WHITE, PieceType::ROOK).pop_count() * Material::ROOK;
    const Score blackRooks =
        position.pieces(Color::BLACK, PieceType::ROOK).pop_count() * Material::ROOK;
    return whiteRooks - blackRooks;
}

Score evaluate_queens(const Position& position)
{
    const Score whiteQueens =
        position.pieces(Color::WHITE, PieceType::QUEEN).pop_count() * Material::QUEEN;
    const Score blackQueens =
        position.pieces(Color::BLACK, PieceType::QUEEN).pop_count() * Material::QUEEN;
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
