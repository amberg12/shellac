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
#include <cstdlib>

#include "engine.h"
#include "movegen.h"
#include "search.h"

#include "evaluate.h"

namespace shellac {
namespace {
constexpr int MAX_DEPTH = 200;
constexpr int MATE_BASE = -MATE_SCORE;

bool is_mate_score(const Score score)
{
    return std::abs(static_cast<int>(score)) >= MATE_BASE - MAX_DEPTH;
}

int to_mate_moves(const Score score)
{
    const int plies = MATE_BASE - std::abs(static_cast<int>(score));
    const int moves = (plies + 1) / 2;
    return score >= 0 ? moves : -moves;
}
} // namespace

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
    hist_ = history;
    hist_.begin_search();
    bestMove_ = Move{};
    stopSearch_.store(false);

    delete timeManager_;
    timeManager_ = new TimeManager(limits, history.pos().side_to_move());

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
    Score alpha = NEG_INF;
    Score beta  = POS_INF;

    auto  moveList  = MoveList::from_position(hist_.pos());
    Score bestScore = NEG_INF;
    Move  bestMove{};

    rescore_moves(moveList);

    for (ScoredMove* move = moveList.begin(); move != moveList.end(); ++move) {
        if (!hist_.pos().is_legal(*move)) {
            continue;
        }

        if (bestMove_.is_null()) {
            bestMove_ = *move;
        }

        if (!allowedRootMoves_.empty()) {
            auto it = std::find(allowedRootMoves_.begin(), allowedRootMoves_.end(),
                                static_cast<Move>(*move));
            if (it == allowedRootMoves_.end()) {
                continue;
            }
        }

        hist_.add_move(static_cast<Move>(*move));
        Score score = -search(depth - 1, -beta, -alpha);
        hist_.pop_move();

        if (stopSearch_.load() == true) {
            return;
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove  = static_cast<Move>(*move);
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break;
        }
    }

    bestMove_ = bestMove;
    std::cout << "info depth " << depth << " score ";
    if (is_mate_score(bestScore)) {
        std::cout << "mate " << to_mate_moves(bestScore);
    }
    else {
        std::cout << "cp " << bestScore;
    }
    std::cout << " nodes " << timeManager_->nodes_searched() << " pv " << bestMove_ << '\n'
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
        return 0;
    }

    if (depth == 0) {
        return quiesce(alpha, beta);
    }

    auto  moveList      = MoveList::from_position(hist_.pos());
    int   searchedMoves = 0;
    Score bestScore     = NEG_INF;

    if (hist_.pos().is_fifty_move() || hist_.pos().is_threefold()) {
        return DRAW_SCORE;
    }

    rescore_moves(moveList);

    for (ScoredMove* move = moveList.begin(); move != moveList.end(); ++move) {
        if (!hist_.pos().is_legal(*move)) {
            continue;
        }

        moveList.pick_move_at(move);

        searchedMoves++;

        hist_.add_move(*move);
        Score score = -search(depth - 1, -beta, -alpha);
        hist_.pop_move();

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
        return hist_.pos().is_check() ? MATE_SCORE + depth : DRAW_SCORE;
    }

    return bestScore;
}

Score Searcher::quiesce(Score alpha, Score beta)
{
    if (stopSearch_.load() == true) {
        return 0;
    }

    TimeManager::Limit limits = timeManager_->check_node();

    if (limits != TimeManager::CONTINUE) {
        stop_searching();
        return 0;
    }

    Score bestScore = hist_.pos().is_check() ? NEG_INF : evaluate(hist_.pos());

    if (hist_.pos().is_fifty_move() || hist_.pos().is_threefold()) {
        return DRAW_SCORE;
    }

    if (bestScore >= beta) {
        return bestScore;
    }

    if (bestScore > alpha) {
        alpha = bestScore;
    }

    int  searchedMoves = 0;
    auto moveList      = hist_.pos().is_check()
             ? MoveList::from_position(hist_.pos())
             : MoveList::from_position<MoveType::CAPTURES>(hist_.pos());

    rescore_moves(moveList);

    for (ScoredMove* move = moveList.begin(); move != moveList.end(); ++move) {
        if (!hist_.pos().is_legal(*move)) {
            continue;
        }

        moveList.pick_move_at(move);

        searchedMoves++;

        hist_.add_move(*move);
        Score score = -quiesce(-beta, -alpha);
        hist_.pop_move();

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
        return hist_.pos().is_check() ? MATE_SCORE : bestScore;
    }

    return bestScore;
}

void Searcher::rescore_moves(MoveList& moveList) const
{
    enum BaseScores : Score
    {
        CAPTURE_BASE = 2000,
    };

    for (ScoredMove& move : moveList) {
        if (move.is_castle()) {
            continue;
        }

        const PieceType victim = type_of(hist_.pos().piece_at(move.dst()));

        if (victim == PieceType::NONE) {
            continue;
        }

        const PieceType attacker = type_of(hist_.pos().piece_at(move.src()));
        move.set_score(evaluate_piece(victim) - evaluate_piece(attacker) + CAPTURE_BASE);
    }
}

} // namespace shellac
