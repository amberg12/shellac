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

#ifndef SHELLAC_MOVEPICKER_H
#define SHELLAC_MOVEPICKER_H

#include "history.h"
#include "movegen.h"

namespace shellac {

struct SearchStack;

class MovePicker
{
public:
    MovePicker() noexcept = default;

    template <MoveType MOVE_TYPE = MoveType::NORMAL>
    MovePicker(const Position& pos);

    template <MoveType MOVE_TYPE = MoveType::NORMAL>
    static MovePicker create(const Position& pos, Move ttMove, SearchStack* ss,
                             QuietHistory& qHist);

    Move next_move();

private:
    Move            moves_[MAX_LEGAL_MOVES]{};
    Score           scores_[MAX_LEGAL_MOVES]{};
    size_t          currentMove_{0};
    const Position* pos_{};
    Move*           end_{};
    Move*           iter_{moves_};
};

template <MoveType MOVE_TYPE>
MovePicker::MovePicker(const Position& pos)
{
    pos_ = &pos;
    end_ = generate_moves<MOVE_TYPE>(moves_, pos);
}

template <MoveType MOVE_TYPE>
MovePicker MovePicker::create(const Position& pos, Move ttMove, SearchStack* ss,
                              QuietHistory& qHist)
{
    enum Bases : Score
    {
        TT_MOVE   = 10'000,
        CAPTURE   = 2'000,
        PROMOTION = CAPTURE,
    };

    MovePicker mp;
    mp.pos_ = &pos;

    mp.end_ = generate_moves<MOVE_TYPE>(mp.moves_, pos);

    for (size_t i = 0; i < mp.end_ - mp.moves_; ++i) {
        Move move = mp.moves_[i];

        if (move == ttMove) {
            mp.scores_[i] = TT_MOVE;
            continue;
        }

        if (pos.is_capture(move)) {
            const PieceType victim =
                move.is_en_passant() ? PieceType::PAWN : type_of(pos.piece_at(move.dst()));
            const PieceType attacker = type_of(pos.piece_at(move.src()));

            mp.scores_[i] = evaluate_piece(victim) - evaluate_piece(attacker) + CAPTURE;
            continue;
        }

        if (move.is_promotion()) {
            const PieceType promoteTo = move.promotion_piece();

            if (promoteTo == PieceType::QUEEN) {
                mp.scores_[i] = QUEEN_SCORE - PAWN_SCORE + PROMOTION;
            } else {
                // We assume underpromotion is probably not the right choice.
                // If underpromotion is good then it is probably best to promote to a knight.
                mp.scores_[i] = -evaluate_piece(promoteTo);
            }
            continue;
        }

        mp.scores_[i] = qHist.read(pos, move);
    }

    return mp;
}

} // namespace shellac

#endif // SHELLAC_MOVEPICKER_H
