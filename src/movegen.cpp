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

#include "movegen.h"

#include "position.h"

namespace shellac {
namespace {

ScoredMove* generate_pawn_moves(ScoredMove* begin, const Position& position)
{
    return begin;
}

template <MoveType MOVE_TYPE, PieceType PIECE_TYPE>
ScoredMove* generate_moves_for(ScoredMove* begin, const Position& position)
{
    for (const Square src : position.pieces(position.side_to_move(), PIECE_TYPE)) {
        const Bitboard attacks = generate_attacks<PIECE_TYPE>(src, position.pieces());
        const Bitboard destinations = attacks & ~position.pieces(position.side_to_move());

        for (const Square dst : destinations) {
            *begin++ = ScoredMove(src, dst);
        }
    }

    return begin;
}

ScoredMove* generate_king_moves(ScoredMove* begin, const Position& position)
{
    return begin;
}

template <MoveType MOVE_TYPE>
ScoredMove* generate_moves(ScoredMove* begin, const Position& position)
{
    begin = generate_pawn_moves(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::KNIGHT>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::BISHOP>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::ROOK>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::QUEEN>(begin, position);
    begin = generate_king_moves(begin, position);
    return begin;
}

} // namespace

template <MoveType Mt>
MoveList MoveList::from_position(const Position& position)
{
    MoveList list{};
    list.end_ = generate_moves<Mt>(list.buffer_, position);
    return list;
}

template MoveList MoveList::from_position<MoveType::NORMAL>(const Position& position);

} // namespace shellac
