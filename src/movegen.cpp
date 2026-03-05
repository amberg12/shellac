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

template <MoveType MOVE_TYPE, Color SIDE_TO_MOVE, PieceType PIECE_TYPE>
std::enable_if_t<is_pawn_v<PIECE_TYPE>, ScoredMove*> generate_moves_for(ScoredMove*     begin,
                                                                        const Position& position)
{
    constexpr Direction DIRECTION = []()
    {
        if constexpr (SIDE_TO_MOVE == Color::WHITE) {
            return Direction::NORTH;
        }

        return Direction::SOUTH;
    }();

    for (const Square src : position.pieces(SIDE_TO_MOVE, PieceType::PAWN)) {
        const Square potentialTarget = src + DIRECTION;
        if (position.piece_at(potentialTarget) == Piece::NONE) {
            *begin++ = ScoredMove{src, potentialTarget};
        }
    }

    return begin;
}

template <MoveType MOVE_TYPE, Color SIDE_TO_MOVE, PieceType PIECE_TYPE>
std::enable_if_t<is_knight_v<PIECE_TYPE>, ScoredMove*> generate_moves_for(ScoredMove*     begin,
                                                                          const Position& position)
{
    return begin;
}

template <MoveType MOVE_TYPE, Color SIDE_TO_MOVE, PieceType PIECE_TYPE>
std::enable_if_t<is_slider_v<PIECE_TYPE>, ScoredMove*> generate_moves_for(ScoredMove*     begin,
                                                                          const Position& position)
{
    return begin;
}

template <MoveType MOVE_TYPE, Color SIDE_TO_MOVE, PieceType PIECE_TYPE>
std::enable_if_t<is_king_v<PIECE_TYPE>, ScoredMove*> generate_moves_for(ScoredMove*     begin,
                                                                        const Position& position)
{
    return begin;
}

template <MoveType MOVE_TYPE, Color SIDE_TO_MOVE>
ScoredMove* generate_legal_moves(ScoredMove* begin, const Position& position)
{
    begin = generate_moves_for<MOVE_TYPE, SIDE_TO_MOVE, PieceType::PAWN>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, SIDE_TO_MOVE, PieceType::KNIGHT>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, SIDE_TO_MOVE, PieceType::BISHOP>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, SIDE_TO_MOVE, PieceType::ROOK>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, SIDE_TO_MOVE, PieceType::QUEEN>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, SIDE_TO_MOVE, PieceType::KING>(begin, position);
    return begin;
}

} // namespace

template <MoveType Mt>
MoveList MoveList::from_position(const Position& position)
{
    MoveList list{};
    if (position.side_to_move() == Color::WHITE) {
        list.end_ = generate_legal_moves<Mt, Color::WHITE>(list.buffer_, position);
    }
    else {
        list.end_ = generate_legal_moves<Mt, Color::BLACK>(list.buffer_, position);
    }
    return list;
}

template MoveList MoveList::from_position<MoveType::LEGAL>(const Position& position);

} // namespace shellac
