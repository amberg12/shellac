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
// Created by amber on 02/03/2026.
//

#ifndef SHELLAC_ENGINE_H
#define SHELLAC_ENGINE_H
#include <vector>

#include "position.h"
#include "search.h"
#include "threadpool.h"
#include "tt.h"

namespace shellac {

struct SearchLimits
{
    std::optional<std::vector<Move>> searchMoves = std::nullopt;

    std::optional<int> whiteTime = std::nullopt;
    std::optional<int> blackTime = std::nullopt;
    std::optional<int> whiteInc  = std::nullopt;
    std::optional<int> blackInc  = std::nullopt;

    std::optional<int> movesToGo = std::nullopt;

    std::optional<int> depth = std::nullopt;
    std::optional<int> nodes = std::nullopt;
    std::optional<int> mate  = std::nullopt;

    std::optional<int> moveTime = std::nullopt;

    bool infinite = false;
};

class Engine
{
public:
    // Normal behaviour.
    void set_position(const std::string& fen, const std::vector<std::string>& moves);
    void go(const SearchLimits& searchLimits);
    void stop();
    void new_game();

    // Our extensions.
    [[nodiscard]] std::string display() const;
    void                      perft(int depth) const;
    void                      perft_suite(int maxDepth, const std::string& path);

private:
    GameHistory               gameHistory_{std::string(STARTING_POSITION), {}};
    std::shared_ptr<Searcher> searcher_ = std::make_shared<Searcher>();
    std::mutex                searcherMutex_;
    ThreadPool                threadPool_;
};

} // namespace shellac

#endif // SHELLAC_ENGINE_H
