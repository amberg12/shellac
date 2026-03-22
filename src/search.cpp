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

#include <cmath>

#include "tt.h"

#include "evaluate.h"
#include "movepicker.h"

namespace shellac {
namespace {
constexpr int SS_OFFSET = 10;
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

Score score_to_tt(const Score score, const int ply)
{
    if (!is_mate_score(score)) {
        return score;
    }

    if (score > 0) {
        return score + ply;
    }
    else {
        return score - ply;
    }
}

Score tt_to_score(const Score score, const int ply)
{
    if (!is_mate_score(score)) {
        return score;
    }

    if (score > 0) {
        return score - ply;
    }
    else {
        return score + ply;
    }
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

std::uint64_t TimeManager::time_elapsed() const
{
    auto elapsed = std::chrono::steady_clock::now() - startTime_;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

void Searcher::begin_search(const GameHistory& history, const SearchLimits& limits)
{
    hist_ = history;
    hist_.begin_search();
    bestMove_ = Move{};
    stopSearch_.store(false);
    quietHistory_ = QuietHistory{};

    tt_.begin_new_search();

    SearchStack  searchStack[SS_OFFSET + MAX_DEPTH * 2]{};
    SearchStack* searchStackRoot = searchStack + 10;

    for (int ply = 0; ply < MAX_DEPTH; ++ply) {
        SearchStack* ss = searchStack + SS_OFFSET + ply;
        ss->ply         = ply;
    }

    delete timeManager_;
    timeManager_ = new TimeManager(limits, history.pos().side_to_move());

    if (limits.searchMoves.has_value()) {
        allowedRootMoves_ = *limits.searchMoves;
    }
    else {
        allowedRootMoves_ = {};
    }

    // Sometimes we try to make a null move, so we load a move in.
    bestMove_ = MovePicker::create(hist_.pos(), Move{}, searchStackRoot, quietHistory_).next_move();

    for (int depth = 1; depth < timeManager_->max_depth(); ++depth) {
        reportData_ = SearchReportData{};

        Score score = search<NodeType::ROOT>(depth, searchStackRoot, NEG_INF, POS_INF);

        if (stopSearch_.load()) {
            break;
        }

        bool isCorruptPv = stopSearch_.load();

        reportData_.pv       = isCorruptPv ? std::vector<Move>{} : searchStackRoot->pv;
        reportData_.nodes    = timeManager_->nodes_searched();
        reportData_.selDepth = std::max(reportData_.depth, reportData_.selDepth);
        reportData_.timeMs   = timeManager_->time_elapsed();
        reportData_.nps =
            reportData_.timeMs > 0 ? (reportData_.nodes * 1000) / reportData_.timeMs : 0;
        if (is_mate_score(score)) {
            reportData_.mate   = true;
            reportData_.mateIn = to_mate_moves(score);
        }
        else {
            reportData_.score = score;
        }
        reportData_.report();
    }

    if (stopSearch_.load() == false) {
        stop_searching();
    }
}

void Searcher::new_game()
{
    tt_.clear();
}

void Searcher::stop_searching()
{
    stopSearch_.store(true);
    std::cout << "bestmove " << bestMove_ << '\n' << std::flush;
}

template <NodeType NODE_TYPE>
Score Searcher::search(int depth, SearchStack* ss, Score alpha, const Score beta)
{
    constexpr bool IS_PV   = NODE_TYPE == NodeType::PV || NODE_TYPE == NodeType::ROOT;
    constexpr bool IS_ROOT = NODE_TYPE == NodeType::ROOT;

    if (stopSearch_.load() == true) {
        return 0;
    }

    TimeManager::Limit limits = timeManager_->check_node();

    if (limits != TimeManager::CONTINUE) {
        stop_searching();
        return 0;
    }

    if (depth <= 0) {
        return quiesce<NodeType::NON_PV>(ss, alpha, beta);
    }

    if (hist_.pos().is_fifty_move() || hist_.pos().is_threefold()) {
        return DRAW_SCORE;
    }

    const Score originalAlpha = alpha;

    auto [ttMove, ttScore, ttDepth, ttTag, ttAge] = tt_.read(hist_.pos());
    if (!ttMove.is_null() && ttDepth >= depth) {
        if constexpr (NODE_TYPE == NodeType::ROOT) {
            if (bestMove_.is_null()) {
                bestMove_ = ttMove;
            }
        }
        else {
            ttScore = tt_to_score(ttScore, ss->ply);

            if (ttTag == TtTag::EXACT) {
                return ttScore;
            }

            if (ttTag == TtTag::LOWER && ttScore >= beta) {
                return ttScore;
            }

            if (ttTag == TtTag::UPPER && ttScore <= alpha) {
                return ttScore;
            }
        }
    }

    const Score staticEval = evaluate(hist_.pos());

    if (!hist_.pos().is_check()) {
        const Score margin = 150 * depth;

        // There are many parameters that can be adjusted here which may gain elo.
        if (staticEval >= beta + margin) {
            return staticEval;
        }

        const bool nmpIsOkNode = !IS_PV && !(ss - 1)->isNull && !ss->isNmpVerification;

        if (nmpIsOkNode && depth >= 3 && staticEval >= beta + 10 * depth) {
            constexpr int r = 4;
            add_move(Move{}, ss, false);
            Score nullMoveScore = -search<NodeType::NON_PV>(depth - r, ss + 1, -beta, -(beta - 1));
            pop_move(ss);

            if (nullMoveScore >= beta) {
                // We must verify the NMP value to avoid zugzwang. Here we stop NMP for a few plies
                // in order to not get stuck reverifying too often.
                int pliesToSkip = std::max(2, (depth - r) * 3 / 4);
                for (int i = 0; i < pliesToSkip; ++i) {
                    (ss + i)->isNmpVerification = true;
                }

                nullMoveScore = search<NodeType::NON_PV>(depth - r, ss, beta - 1, beta);

                for (int i = 0; i < pliesToSkip; ++i) {
                    (ss + i)->isNmpVerification = false;
                }

                if (nullMoveScore >= beta) {
                    return nullMoveScore;
                }
            }
        }
    }

    auto  mp            = MovePicker::create(hist_.pos(), ttMove, ss, quietHistory_);
    int   searchedMoves = 0;
    Score bestScore     = NEG_INF;
    Move  bestMove      = Move{};

    Move searchedQuiets[MAX_LEGAL_MOVES]{};
    int  searchedQuietCount = 0;

    Move move;
    while (!(move = mp.next_move()).is_null()) {
        if (NODE_TYPE == NodeType::ROOT && stopSearch_.load()) {
            break;
        }

        searchedMoves++;

        bool isQuiet = !(move.is_promotion() || hist_.pos().is_capture(move));

        if (isQuiet) {
            searchedQuiets[searchedQuietCount++] = move;
        }

        add_move(move, ss, false);

        Score score = NEG_INF;
        if (depth >= 3 && searchedMoves > 1) {
            int r = 1 + std::log(depth) * std::log(searchedMoves) / 3;

            // Tactical positions need more care.
            if (hist_.pos().is_capture(move)) {
                r -= 1;
            }

            r = std::clamp(r, 0, depth - 1);

            score = -search<NodeType::NON_PV>(depth - r - 1, ss + 1, -alpha - 1, -alpha);

            if (r >= 1 && score > alpha) {
                score = -search<NodeType::NON_PV>(depth - 1, ss + 1, -alpha - 1, -alpha);
            }
        }
        else if (!IS_PV || searchedMoves != 1) {
            score = -search<NodeType::NON_PV>(depth - 1, ss + 1, -alpha - 1, -alpha);
        }

        if (IS_PV && (searchedMoves == 1 || score > alpha)) {
            score = -search<NodeType::PV>(depth - 1, ss + 1, -beta, -alpha);
        }

        pop_move(ss);

        if (score > bestScore) {
            bestScore = score;
            bestMove  = move;

            ss->pv.clear();
            ss->pv.push_back(move);
            for (const auto& m : (ss + 1)->pv) {
                if (m.is_null()) {
                    continue;
                }

                ss->pv.push_back(m);
            }
        }

        if (score > alpha) {
            alpha = score;
        }

        if (score >= beta) {
            if (isQuiet) {
                quietHistory_.write(hist_.pos(), move, depth * depth);

                for (int i = 0; i < searchedQuietCount - 1; ++i) {
                    // The "correct" formula is -depth * depth, but people on Discord have
                    // suggested this as working better in weaker engines.
                    quietHistory_.write(hist_.pos(), searchedQuiets[i], -depth);
                }
            }

            break;
        }
    }

    if (searchedMoves == 0) {
        bestScore = hist_.pos().is_check() ? MATE_SCORE + ss->ply : DRAW_SCORE;
    }

    TtTag writeTag = TtTag::EXACT;
    if (bestScore <= originalAlpha) {
        writeTag = TtTag::UPPER;
    }
    else if (bestScore >= beta) {
        writeTag = TtTag::LOWER;
    }

    const Score writeScore = score_to_tt(bestScore, ss->ply);
    if (!stopSearch_.load()) {
        tt_.write(hist_.pos(), bestMove, writeScore, depth, writeTag);
    }

    if constexpr (NODE_TYPE == NodeType::ROOT) {
        bestMove_ = bestMove;
    }

    return bestScore;
}

template <NodeType NODE_TYPE>
Score Searcher::quiesce(SearchStack* ss, Score alpha, Score beta)
{
    static_assert(NODE_TYPE != NodeType::ROOT);

    constexpr bool IS_PV = NODE_TYPE == NodeType::PV || NODE_TYPE == NodeType::ROOT;

    if (stopSearch_.load() == true) {
        return 0;
    }

    TimeManager::Limit limits = timeManager_->check_node();

    if (limits != TimeManager::CONTINUE) {
        stop_searching();
        return 0;
    }

    const Score originalAlpha = alpha;

    auto [ttMove, ttScore, ttDepth, ttTag, _] = tt_.read(hist_.pos());
    if (!ttMove.is_null()) {
        if constexpr (NODE_TYPE == NodeType::ROOT) {
            if (bestMove_.is_null()) {
                bestMove_ = ttMove;
            }
        }

        ttScore = tt_to_score(ttScore, ss->ply);

        if (ttTag == TtTag::EXACT) {
            return ttScore;
        }

        if (ttTag == TtTag::LOWER && ttScore >= beta) {
            return ttScore;
        }

        if (ttTag == TtTag::UPPER && ttScore <= alpha) {
            return ttScore;
        }
    }

    Score bestScore = hist_.pos().is_check() ? NEG_INF : evaluate(hist_.pos());
    Move  bestMove  = Move{};

    if (hist_.pos().is_fifty_move() || hist_.pos().is_threefold()) {
        return DRAW_SCORE;
    }

    if (bestScore >= beta) {
        return bestScore;
    }

    if (bestScore > alpha) {
        alpha = bestScore;
    }

    int searchedMoves = 0;

    MovePicker mp; // default-construct first
    if (hist_.pos().is_check()) {
        mp = MovePicker::create(hist_.pos(), ttMove, ss, quietHistory_);
    }
    else {
        mp = MovePicker::create<MoveType::CAPTURES>(hist_.pos(), ttMove, ss, quietHistory_);
    }

    Move move;
    while (!(move = mp.next_move()).is_null()) {

        searchedMoves++;

        add_move(move, ss, true);

        Score score;
        if (searchedMoves == 1) {
            constexpr NodeType N = IS_PV ? NodeType::PV : NodeType::NON_PV;
            score                = -quiesce<N>(ss + 1, -beta, -alpha);
        }
        else {
            score = -quiesce<NodeType::NON_PV>(ss + 1, -alpha - 1, -alpha);
            if (score > alpha && IS_PV) {
                score = -quiesce<NodeType::PV>(ss + 1, -beta, -alpha);
            }
        }

        pop_move(ss);

        if (score > bestScore) {
            bestScore = score;
            bestMove  = move;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (score >= beta) {
            break;
        }
    }

    if (searchedMoves == 0) {
        bestScore = hist_.pos().is_check() ? MATE_SCORE + ss->ply : bestScore;
    }

    TtTag writeTag = TtTag::EXACT;
    if (bestScore <= originalAlpha) {
        writeTag = TtTag::UPPER;
    }
    else if (bestScore >= beta) {
        writeTag = TtTag::LOWER;
    }

    const Score writeScore = score_to_tt(bestScore, ss->ply);
    if (!stopSearch_.load()) {
        tt_.write(hist_.pos(), bestMove, writeScore, 0, writeTag);
    }

    return bestScore;
}

void Searcher::add_move(const Move move, SearchStack* ss, bool isSel)
{
    hist_.add_move(move);
    ss->playedMove = move;
    ss->isNull     = move.is_null();
    (ss + 1)->pv.clear();

    if (isSel) {
        reportData_.selDepth = std::max(reportData_.selDepth, (ss + 1)->ply);
    }
    else {
        reportData_.depth = std::max(reportData_.depth, (ss + 1)->ply);
    }
}

void Searcher::pop_move(SearchStack* ss)
{
    hist_.pop_move();
    (void)ss;
}
} // namespace shellac
