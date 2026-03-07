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
// Created by amber on 07/03/2026.
//

#ifndef SHELLAC_SEARCH_H
#define SHELLAC_SEARCH_H
#include <atomic>

#include "position.h"
#include "types.h"

namespace shellac {

class SearchLimits;

class Searcher
{
public:
    Searcher() = default;

    void begin_search(const GameHistory& history, const SearchLimits& limits);
    void stop_searching();

private:
    GameHistory gameHistory_{
        std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), {}};
    Move bestMove_{};

    std::atomic_bool stopSearch_{false};
};

} // namespace shellac

#endif // SHELLAC_SEARCH_H
