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

#include <algorithm>

#include "engine.h"
#include "movegen.h"
#include "search.h"

namespace shellac {
namespace {
constexpr int MAX_DEPTH = 200;
}

TimeManager::TimeManager(const SearchLimits& searchLimits, const Color sideToMove)
{
    startTime_ = std::chrono::steady_clock::now();

    if (searchLimits.depth.has_value()) {
        maxDepth_ = searchLimits.depth.value();
    }

    if (searchLimits.mate.has_value()) {
        maxDepth_ = searchLimits.mate.value() * 2;
    }

    if (searchLimits.nodes.has_value()) {
        maxNodes_ = searchLimits.nodes.value();
    }

    if (searchLimits.infinite) {
        endTime_ = startTime_ + std::chrono::hours(24);
    }
    else if (searchLimits.moveTime.has_value()) {
        endTime_ = startTime_ + std::chrono::milliseconds(searchLimits.moveTime.value());
    }
    else if (searchLimits.whiteTime.has_value() || searchLimits.blackTime.has_value()) {
        int remainingTime = 0;
        int increment     = 0;

        if (sideToMove == Color::WHITE) {
            if (searchLimits.whiteTime.has_value()) {
                remainingTime = searchLimits.whiteTime.value();
            }
            if (searchLimits.whiteInc.has_value()) {
                increment = searchLimits.whiteInc.value();
            }
        }
        else {
            if (searchLimits.blackTime.has_value()) {
                remainingTime = searchLimits.blackTime.value();
            }
            if (searchLimits.blackInc.has_value()) {
                increment = searchLimits.blackInc.value();
            }
        }

        int allocatedTime = remainingTime;

        if (searchLimits.movesToGo.has_value()) {
            allocatedTime = remainingTime / searchLimits.movesToGo.value();
        }
        else {
            allocatedTime = remainingTime / 30;
        }

        allocatedTime += increment;

        endTime_ = startTime_ + std::chrono::milliseconds(allocatedTime);
    }
    else {
        endTime_ = startTime_ + std::chrono::seconds(10);
    }
}

TimeManager::Limit TimeManager::check_node()
{
    using namespace std::chrono_literals;

    ++nodesSearched_;
    if (maxNodes_ != -1) {
        if (nodesSearched_ >= maxNodes_) {
            return HARD_STOP;
        }
    }

    if (nodesSearched_ % 4096 != 0) {
        return CONTINUE;
    }

    if (std::chrono::steady_clock::now() >= endTime_ - 50ms) {
        return SOFT_STOP;
    }

    return CONTINUE;
}

int TimeManager::nodes_searched() const
{
    return nodesSearched_;
}

int TimeManager::max_depth() const
{
    return maxDepth_ == -1 ? MAX_DEPTH : maxDepth_;
}

void Searcher::begin_search(const GameHistory& history, const SearchLimits& limits)
{
    gameHistory_ = history;
    bestMove_    = Move{};
    stopSearch_.store(false);

    delete timeManager_;
    timeManager_ = new TimeManager(limits, history.current_position().side_to_move());

    if (limits.searchMoves.has_value()) {
        allowedRootMoves_ = *limits.searchMoves;
    }
    else {
        allowedRootMoves_ = {};
    }

    for (int depth = 1; depth < timeManager_->max_depth(); ++depth) {
        search_root(depth);
    }

    if (stopSearch_.load() == false) {
        stop_searching();
    }
}

void Searcher::stop_searching()
{
    stopSearch_.store(true);
    std::cout << "bestmove " << bestMove_ << '\n' << std::flush;
}

void Searcher::search_root(int depth)
{
    const Position& currentPosition = gameHistory_.current_position();
    Score           alpha           = NEG_INF;
    Score           beta            = POS_INF;

    const auto moveList  = MoveList::from_position(currentPosition);
    Score      bestScore = NEG_INF;
    Move       bestMove{};

    for (const auto& move : moveList) {
        if (!currentPosition.is_legal(static_cast<Move>(move))) {
            continue;
        }

        if (!allowedRootMoves_.empty()) {
            auto it = std::find(allowedRootMoves_.begin(), allowedRootMoves_.end(),
                                static_cast<Move>(move));
            if (it == allowedRootMoves_.end()) {
                continue;
            }
        }

        gameHistory_.add_move(static_cast<Move>(move));
        Score score = -search(depth - 1, -beta, -alpha);
        gameHistory_.pop_move();

        if (stopSearch_.load() == true) {
            return;
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove  = static_cast<Move>(move);
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break;
        }
    }

    bestMove_ = bestMove;
    std::cout << "info depth " << depth << " score cp " << bestScore << " nodes "
              << timeManager_->nodes_searched() << " pv " << bestMove_ << '\n'
              << std::flush;
}

Score Searcher::search(int depth, Score alpha, const Score beta)
{
    if (stopSearch_.load() == true) {
        return 0;
    }

    TimeManager::Limit limits = timeManager_->check_node();

    if (limits != TimeManager::CONTINUE) {
        stop_searching();
    }

    if (depth == 0) {
        return rand() % 200 - 100;
    }

    const Position& currentPosition = gameHistory_.current_position();
    const auto      moveList        = MoveList::from_position(currentPosition);
    int             searchedMoves   = 0;
    Score           bestScore       = NEG_INF;

    if (currentPosition.is_fifty_move() || currentPosition.is_threefold()) {
        return DRAW_SCORE;
    }

    for (const Move move : moveList) {
        if (!currentPosition.is_legal(move)) {
            continue;
        }

        searchedMoves++;

        gameHistory_.add_move(move);
        Score score = -search(depth - 1, -beta, -alpha);
        gameHistory_.pop_move();

        if (score > bestScore) {
            bestScore = score;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (score >= beta) {
            break;
        }
    }

    if (searchedMoves == 0) {
        return currentPosition.is_check() ? MATE_SCORE + depth : DRAW_SCORE;
    }

    return bestScore;
}

} // namespace shellac
