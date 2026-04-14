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

#ifndef SHELLAC_HISTORY_H
#define SHELLAC_HISTORY_H
#include <algorithm>

#include "position.h"
#include "types.h"

namespace shellac {

constexpr Score kHistoryMax = 16384;

constexpr Score history_bonus(int depth)
{
    return std::clamp(320 * depth - 400, 0, 2400);
}

constexpr Score history_malus(int depth)
{
    return -std::clamp(320 * depth - 400, 0, 1200);
}

inline void update_with_gravity(Score* score, Score base, Score bonus)
{
    bonus = std::clamp(bonus, Score(-kHistoryMax), kHistoryMax);
    *score += bonus - base * std::abs(bonus) / kHistoryMax;
}

template <typename T>
class ButterflyTable
{
public:
    ButterflyTable() = default;

    T* read(const Position& pos, Move move)
    {
        size_t colorIdx = underlying(pos.side_to_move());
        size_t srcIdx   = underlying(move.src());
        size_t dstIdx   = underlying(move.dst());

        return &butterflyTable_[colorIdx][srcIdx][dstIdx];
    }

    const T* read(const Position& pos, Move move) const
    {
        size_t colorIdx = underlying(pos.side_to_move());
        size_t srcIdx   = underlying(move.src());
        size_t dstIdx   = underlying(move.dst());

        return &butterflyTable_[colorIdx][srcIdx][dstIdx];
    }

private:
    T butterflyTable_[2][64][64]{};
};

using ButterflyHistory = ButterflyTable<Score>;

inline void update_butterfly_history(ButterflyHistory& butterflyTable, const Position& pos,
                                     Move move, Score bonus)
{
    Score* entry = butterflyTable.read(pos, move);
    update_with_gravity(entry, *entry, bonus);
}

inline Score read_butterfly_history(const ButterflyHistory& butterflyTable, const Position& pos,
                                    Move move)
{
    return *butterflyTable.read(pos, move);
}

} // namespace shellac

#endif // SHELLAC_HISTORY_H
