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
#include "position.h"
#include "types.h"

namespace shellac {

class QuietHistory
{
public:
    QuietHistory() = default;

    [[nodiscard]] Score read(const Position& pos, Move move) const;
    void                write(const Position& pos, Move move, Score bonus);

private:
    Score& get_butterfly_score(const Position& pos, Move move);
    const Score& get_butterfly_score(const Position& pos, Move move) const;

    Score butterflyTable_[2][64][64]{};
};

} // namespace shellac

#endif // SHELLAC_HISTORY_H
