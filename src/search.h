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

#include "history.h"
#include "position.h"
#include "tt.h"
#include "types.h"

namespace shellac {
class TranspositionTable;

struct SearchLimits;

struct SearchStack
{
    int               ply{};
    Move              playedMove;
    bool              isNull{false};
    bool              isNmpVerification{false};
    std::vector<Move> pv;
    Score             staticEval{NO_SCORE};
};

struct SearchReportData
{
    int depth    = 0;
    int selDepth = 0;

    std::vector<Move> pv;

    int  score  = 0;
    bool mate   = false;
    int  mateIn = 0;

    std::uint64_t nodes  = 0;
    std::uint64_t nps    = 0;
    std::uint64_t timeMs = 0;

    void report() const
    {
        std::cout << "info"
                  << " depth " << depth << " seldepth " << selDepth;

        if (mate) {
            std::cout << " score mate " << mateIn;
        }
        else {
            std::cout << " score cp " << score;
        }

        std::cout << " nodes " << nodes << " nps " << nps << " time " << timeMs;

        if (!pv.empty()) {
            std::cout << " pv";
            for (const auto& move : pv) {
                std::cout << " " << move;
            }
        }

        std::cout << std::endl;
    }
};
enum class NodeType
{
    ROOT,
    PV,
    NON_PV,
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

    [[nodiscard]] int           nodes_searched() const;
    [[nodiscard]] int           max_depth() const;
    [[nodiscard]] std::uint64_t time_elapsed() const;

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
    void new_game();
    void begin_search(const GameHistory& history, const SearchLimits& limits);
    void stop_searching();

    void set_hash_size(size_t mb);

private:
    template <NodeType NODE_TYPE>
    Score search(int depth, SearchStack* ss, Score alpha, Score beta);
    template <NodeType NODE_TYPE>
    Score quiesce(SearchStack* ss, Score alpha, Score beta);

    void add_move(Move move, SearchStack* ss, bool isSel);
    void pop_move(SearchStack* ss);

    TimeManager* timeManager_ = nullptr;
    GameHistory  hist_{std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), {}};
    Move         bestMove_{};

    std::vector<Move> allowedRootMoves_{};

    TranspositionTable tt_{16};
    QuietHistory       quietHistory_{};

    SearchReportData reportData_{};

    std::atomic_bool stopSearch_{false};
};

} // namespace shellac

#endif // SHELLAC_SEARCH_H
