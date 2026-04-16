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

#include "movepicker.h"

namespace shellac {
namespace {
MovePickerStage get_initial_move_picker_stage(MovePickerType movePickerType)
{
    return movePickerType == MovePickerType::kNormal ? MovePickerStage::kTtMove
                                                     : MovePickerStage::kQsTtMove;
}

MovePickerStage& operator++(MovePickerStage& movePickerStage)
{
    movePickerStage = static_cast<MovePickerStage>(underlying(movePickerStage) + 1);
    return movePickerStage;
}
} // namespace

MovePicker::MovePicker(MovePickerType movePickerType, const Position& pos, Move ttMove,
                       SearchStack* ss, const ButterflyHistory& butterflyHistory) :
    movePickerStage_{get_initial_move_picker_stage(movePickerType)}, pos_{pos}, ttMove_{ttMove},
    ss_{ss}, butterflyHistory_{butterflyHistory}
{
}

Move MovePicker::next_move()
{
    switch (movePickerStage_) {
    case MovePickerStage::kTtMove:
    {
        ++movePickerStage_;
        if (!ttMove_.is_null()) {
            return ttMove_;
        }
        [[fallthrough]];
    }
    case MovePickerStage::kGenerateMoves:
    {
        ++movePickerStage_;

        Move* end           = generate_moves<MoveType::kNormal>(generatedMoves_.data(), pos_);
        generatedMoveCount_ = end - generatedMoves_.data();
        score_moves();

        [[fallthrough]];
    }
    case MovePickerStage::kPickMoves:
    {
        Move move = pick_next_move();

        return move;
    }
    case MovePickerStage::kQsTtMove:
    {
        ++movePickerStage_;
        if (!ttMove_.is_null() && pos_.is_capture(ttMove_)) {
            return ttMove_;
        }

        [[fallthrough]];
    }
    case MovePickerStage::kQsGenerateMoves:
    {
        ++movePickerStage_;

        Move* end           = generate_moves<MoveType::kCaptures>(generatedMoves_.data(), pos_);
        generatedMoveCount_ = end - generatedMoves_.data();
        score_moves();

        [[fallthrough]];
    }
    case MovePickerStage::kQsPickMoves:
    {
        Move move = pick_next_move();

        return move;
    }
    }

    return kNullMove;
}

void MovePicker::score_moves()
{
    enum MoveOffsets : i32
    {
        kCapture   = 800'000'000,
        kPromotion = 700'000'000,
        kKiller    = 600'000'000,
    };

    for (usize i = 0; i < generatedMoveCount_; ++i) {
        Move move = generatedMoves_[i];

        if (move.is_promotion()) {
            if (move.promotion_piece() == PieceType::QUEEN) {
                generatedScores_[i] = kPromotion;
            }
            else {
                generatedScores_[i] = -kPromotion;
            }

            continue;
        }

        if (pos_.is_capture(move)) {
            PieceType mvv =
                move.is_en_passant() ? PieceType::PAWN : type_of(pos_.piece_at(move.dst()));
            PieceType lva = type_of(pos_.piece_at(move.src()));

            generatedScores_[i] = kCapture + 20 * evaluate_piece(mvv) - evaluate_piece(lva);
            continue;
        }

        if (move == ss_->killer) {
            generatedScores_[i] = kKiller;
            continue;
        }

        generatedScores_[i] = read_butterfly_history(butterflyHistory_, pos_, move);
    }
}

Move MovePicker::pick_next_move()
{
    while (currentMove_ < generatedMoveCount_) {
        // Find the highest-scoring move in the unsorted suffix and swap it into place.
        usize best = currentMove_;
        for (usize i = currentMove_ + 1; i < generatedMoveCount_; ++i) {
            if (generatedScores_[i] > generatedScores_[best]) {
                best = i;
            }
        }
        std::swap(generatedMoves_[currentMove_], generatedMoves_[best]);
        std::swap(generatedScores_[currentMove_], generatedScores_[best]);

        Move move = generatedMoves_[currentMove_++];

        if (move == ttMove_ || !pos_.is_legal(move)) {
            continue;
        }

        return move;
    }

    return kNullMove;
}
} // namespace shellac
