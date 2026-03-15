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
#include <chrono>

#include "engine.h"
#include "movegen.h"
#include "position.h"
#include "types.h"

namespace shellac {
class TranspositionTable;

struct SearchLimits;

struct SearchStack
{
    int  ply;
    Move playedMove;
};

enum class NodeType
{
    ROOT,
    PV,
    STANDARD,
    NULL_MOVE_SEARCH,
};

class TimeManager
{
public:
    enum Limit
    {
        CONTINUE,
        HARD_STOP,
        SOFT_STOP,
    };

    TimeManager(const SearchLimits& searchLimits, Color color);

    [[nodiscard]] Limit check_node();

    [[nodiscard]] int nodes_searched() const;
    [[nodiscard]] int max_depth() const;

private:
    std::chrono::steady_clock::time_point startTime_{};
    std::chrono::steady_clock::time_point endTime_{};

    int          nodesSearched_ = 0;
    int          maxDepth_      = -1;
    std::int64_t maxNodes_      = -1;
};

class Searcher
{
public:
    Searcher() = default;

    void begin_search(const GameHistory& history, const SearchLimits& limits,
                      TranspositionTable* tt);
    void stop_searching();

private:
    template <NodeType NODE_TYPE>
    Score search(int depth, SearchStack* ss, Score alpha, Score beta);
    template <NodeType NODE_TYPE>
    Score quiesce(SearchStack* ss, Score alpha, Score beta);

    void add_move(Move move, SearchStack* ss);
    void pop_move(SearchStack* ss);

    void rescore_moves(MoveList& moveList, Move bestMove) const;

    TimeManager* timeManager_ = nullptr;
    GameHistory  hist_{std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), {}};
    Move         bestMove_{};

    std::vector<Move> allowedRootMoves_{};

    TranspositionTable* tt_ = nullptr;
    Score               butterflyTable_[2][64][64]{};

    size_t rootPly_;

    std::atomic_bool stopSearch_{false};
};

} // namespace shellac

#endif // SHELLAC_SEARCH_H
