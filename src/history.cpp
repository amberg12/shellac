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

namespace shellac {
Score QuietHistory::read(const Position& pos, const Move move) const
{
    const size_t colorIndex = underlying(pos.side_to_move());
    const size_t srcIndex   = underlying(move.src());
    const size_t dstIndex   = underlying(move.dst());

    return butterflyTable_[colorIndex][srcIndex][dstIndex];
}

void QuietHistory::write(const Position& pos, const Move move, const Score bonus)
{
    const size_t colorIndex = underlying(pos.side_to_move());
    const size_t srcIndex   = underlying(move.src());
    const size_t dstIndex   = underlying(move.dst());

    butterflyTable_[colorIndex][srcIndex][dstIndex] = bonus;
}

} // namespace shellac
