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

#include "search.h"

#include "movegen.h"

namespace shellac {
void Searcher::begin_search(const GameHistory& history, const SearchLimits& limits)
{
    gameHistory_ = history;
    bestMove_    = Move{};
    (void)limits;
    const auto   moveList  = MoveList::from_position(history.current_position());
    const size_t moveCount = moveList.size();
    size_t       moveIndex = rand() % moveCount;
    auto         it        = moveList.begin();

    while (moveIndex-- || bestMove_.is_null()) {
        if (it == moveList.end()) {
            break;
        }

        const Move thisMove = static_cast<const Move&>(*it++);

        if (history.current_position().is_legal(thisMove)) {
            bestMove_ = thisMove;
        }
    }

    stop_searching();
}

void Searcher::stop_searching()
{
    stopSearch_.store(true);
    std::cout << "bestmove " << bestMove_ << '\n' << std::flush;
}
} // namespace shellac
