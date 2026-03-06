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
// Created by amber on 04/03/2026.
//

#ifndef SHELLAC_MOVEGEN_H
#define SHELLAC_MOVEGEN_H

#include "types.h"

namespace shellac {
class Position;

enum class MoveType
{
    NORMAL,
};

class ScoredMove : public Move
{
public:
    ScoredMove() : Move(Move())
    {
    }
    ScoredMove(const Square src, const Square dst) : Move(src, dst)
    {
    }
    explicit ScoredMove(const Move move) : Move(move)
    {
    }

    ScoredMove(const Move move, const Score score) : Move(move), score_(score)
    {
    }

    static ScoredMove create_en_passant(const Square src, const Square dst)
    {
        return ScoredMove(Move::create_en_passant(src, dst));
    }

    static ScoredMove create_promotion(const Square src, const Square dst, const PieceType pieceType)
    {
        return ScoredMove(Move::create_promotion(src, dst, pieceType));
    }

    static ScoredMove create_castle(const Square src, const Square dst)
    {
        return ScoredMove(Move::create_castle(src, dst));
    }

    void set_score(const Score score)
    {
        score_ = score;
    }

private:
    Score score_{};
};

class MoveList
{
public:
    template <MoveType Mt = MoveType::NORMAL>
    static MoveList from_position(const Position& position);

    [[nodiscard]] ScoredMove* begin()
    {
        return buffer_;
    }

    [[nodiscard]] ScoredMove* end()
    {
        return end_;
    }

    [[nodiscard]] const ScoredMove* begin() const
    {
        return buffer_;
    }

    [[nodiscard]] const ScoredMove* end() const
    {
        return end_;
    }

    [[nodiscard]] size_t size() const
    {
        return end_ - buffer_;
    }

private:
    MoveList() = default;

    static constexpr size_t MAX_LEGAL_MOVES = 267;
    ScoredMove              buffer_[MAX_LEGAL_MOVES];
    ScoredMove*             end_{buffer_};
};

} // namespace shellac

#endif // SHELLAC_MOVEGEN_H
