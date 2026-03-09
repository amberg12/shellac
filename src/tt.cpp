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

#include "tt.h"

#include <strings.h>

#include "position.h"

namespace shellac {
struct TtEntry
{
    std::size_t hash;
    TtData      data;
};

TranspositionTable::TranspositionTable(const size_t mb)
{
    resize(mb);
    clear();
}

void TranspositionTable::clear()
{
    for (size_t i = 0; i < entryCount_; ++i) {
        entryList_[i] = TtEntry();
    }
}

void TranspositionTable::resize(const size_t mb)
{
    const size_t kb    = mb * 1024;
    const size_t bytes = kb * 1024;
    entryCount_        = bytes / sizeof(*entryList_);

    delete[] entryList_;
    entryList_ = new TtEntry[entryCount_];
}

TtData TranspositionTable::read(const Position& position) const
{
    const size_t   index = position.hash() % entryCount_;
    const TtEntry& entry = entryList_[index];

    if (entry.hash != position.hash()) {
        return {};
    }

    return entry.data;
}

void TranspositionTable::write(const Position& position, Move move, Score score, std::uint8_t depth,
                               TtTag tag)
{
    if (move.is_null()) {
        return;
    }

    const TtData data = {
        .move  = move,
        .score = score,
        .depth = depth,
        .tag   = tag,
        .age   = age_,
    };

    const std::uint64_t index        = position.hash() % entryCount_;
    const TtEntry&      currentEntry = entryList_[index];
    const TtData&       currentData  = currentEntry.data;

    if (currentData.age != age_ || (currentData.depth >= depth && !(currentData.depth == 255))) {
        entryList_[index] = {
            .hash = position.hash(),
            .data = data,
        };
    }
}

void TranspositionTable::begin_new_search()
{
    ++age_;
}
} // namespace shellac
