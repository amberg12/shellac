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
#include "search.h"

namespace shellac {
enum class MovePickerType
{
    kNormal,
    kQuiesce,
};

enum class MovePickerStage
{
    kTtMove,
    kGenerateMoves,
    kPickMoves,

    kQsTtMove,
    kQsGenerateMoves,
    kQsPickMoves,
};

class MovePicker
{
public:
    MovePicker(MovePickerType movePickerType, const Position& pos, Move ttMove, SearchStack* ss,
               const ButterflyHistory& butterflyHistory);

    Move next_move();

private:
    void score_moves();
    Move pick_next_move();

    MovePickerStage         movePickerStage_;
    const Position&         pos_;
    Move                    ttMove_;
    SearchStack*            ss_;
    const ButterflyHistory& butterflyHistory_;

    std::array<Move, kMaxLegalMoves> generatedMoves_;
    std::array<i32, kMaxLegalMoves>  generatedScores_;
    usize                            generatedMoveCount_{};
    usize                            currentMove_{};
};
} // namespace shellac

#endif // SHELLAC_MOVEPICKER_H
