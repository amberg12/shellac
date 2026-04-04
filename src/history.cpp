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

#include "history.h"

#include <algorithm>

namespace shellac {
namespace {
void apply_gravity(Score& score, Score bonus)
{
    constexpr Score HISTORY_MAX = 1500;

    Score clampedBonus = std::clamp(bonus, Score(-HISTORY_MAX), Score(HISTORY_MAX));
    score += clampedBonus - score * std::abs(clampedBonus) / HISTORY_MAX;
}
}

Score QuietHistory::read(const Position& pos, const Move move) const
{
    return get_butterfly_score(pos, move);
}

void QuietHistory::write(const Position& pos, const Move move, const Score bonus)
{
    Score& butterflyScore = get_butterfly_score(pos, move);
    apply_gravity(butterflyScore, bonus);
}

Score& QuietHistory::get_butterfly_score(const Position& pos, Move move)
{
    const size_t colorIndex = underlying(pos.side_to_move());
    const size_t srcIndex   = underlying(move.src());
    const size_t dstIndex   = underlying(move.dst());

    return butterflyTable_[colorIndex][srcIndex][dstIndex];
}

const Score& QuietHistory::get_butterfly_score(const Position& pos, Move move) const
{
    const size_t colorIndex = underlying(pos.side_to_move());
    const size_t srcIndex   = underlying(move.src());
    const size_t dstIndex   = underlying(move.dst());

    return butterflyTable_[colorIndex][srcIndex][dstIndex];
}

} // namespace shellac
