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
// Created by amber on 09/03/2026.
//

#ifndef SHELLAC_TT_H
#define SHELLAC_TT_H
#include "types.h"

namespace shellac {
struct TtEntry;
class Position;

enum class TtTag : std::uint8_t
{
    EMPTY = 0,
    EXACT,
    UPPER,
    LOWER,
};

struct TtData
{
    Move         move;
    Score        score;
    std::uint8_t depth;
    TtTag        tag;
    std::uint8_t age;
};

class TranspositionTable
{
public:
    explicit TranspositionTable(size_t mb);

    void clear();
    void resize(size_t mb);

    TtData read(const Position& position) const;
    void   write(const Position& position, Move move, Score score, std::uint8_t depth, TtTag tag);
    void   begin_new_search();

private:
    TtEntry* entryList_{nullptr};
    size_t   entryCount_{0};

    std::uint8_t age_;
};

} // namespace shellac

#endif // SHELLAC_TT_H
