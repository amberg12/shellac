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
#include "util.h"

namespace shellac {
namespace {
constexpr int SS_OFFSET = 10;
constexpr int MAX_DEPTH = 200;
constexpr int MATE_BASE = -MATE_SCORE;

bool is_improving(const SearchStack* ss)
{
    if (ss->staticEval == NO_SCORE) {
        return false;
    }

    if ((ss - 2)->staticEval != NO_SCORE) {
        return ss->staticEval > (ss - 2)->staticEval;
    }

    if ((ss - 4)->staticEval != NO_SCORE) {
        return ss->staticEval > (ss - 4)->staticEval;
    }

    return true;
}

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
            allocatedTime = remainingTime / (searchLimits.movesToGo.value() + 1);
        }
        else {
            allocatedTime = remainingTime / 30;
        }

        allocatedTime += increment;

        endTime_ = startTime_ + std::chrono::milliseconds(allocatedTime);
    }
    else {
        endTime_ = startTime_ + std::chrono::seconds(999999999);
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
    bestMove_ =
        MovePicker::create(hist_.pos(), Move{}, searchStackRoot, butterflyHistory_).next_move();

    auto  start = std::chrono::high_resolution_clock::now();
    Score score{};
    for (int depth = 1; depth <= timeManager_->max_depth(); ++depth) {
        rootDepth_  = depth;
        reportData_ = SearchReportData{};

        if (depth <= 5 || is_mate_score(score)) {
            score = search<NodeType::ROOT>(depth, searchStackRoot, NEG_INF, POS_INF, false);
        }
        else {
            Score  delta = 8 + score / 64;
            Score  alpha = score - delta;
            Score  beta  = score + delta;
            Bounds bound{};
            do {
                score = search<NodeType::ROOT>(depth, searchStackRoot, alpha, beta, false);

                bound = stopSearch_.load() ? Bounds::EXACT
                    : score >= beta        ? Bounds::LOWER
                    : score <= alpha       ? Bounds::UPPER
                                           : Bounds::EXACT;

                if (bound == Bounds::UPPER) {
                    beta  = (alpha + beta) / 2;
                    alpha = score - delta;
                    alpha = std::clamp(alpha, NEG_INF, POS_INF);
                }
                else if (bound == Bounds::LOWER) {
                    beta = score + delta;
                    beta = std::clamp(beta, NEG_INF, POS_INF);
                }

                delta *= 2;
            }
            while (bound != Bounds::EXACT);
        }

        auto end      = std::chrono::high_resolution_clock::now();
        auto duration = end - start;
        auto us       = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        reportData_.nps = static_cast<std::uint64_t>(timeManager_->nodes_searched()) *
            1'000'000ULL / std::max(std::uint64_t(us), std::uint64_t(1));
        reportData_.score  = score;
        reportData_.nodes  = timeManager_->nodes_searched();
        reportData_.pv     = searchStackRoot->pv;
        reportData_.depth  = depth;
        reportData_.timeMs = us / 1000;
        reportData_.report();

        if (stopSearch_.load()) {
            break;
        }
    }

    if (stopSearch_.load() == false) {
        stop_searching();
    }

    std::cout << "bestmove " << to_string(bestMove_) << std::endl;
}

void Searcher::new_game()
{
    tt_.clear();
    butterflyHistory_ = ButterflyHistory{};
}

void Searcher::stop_searching()
{
    stopSearch_.store(true);
}

void Searcher::set_hash_size(size_t mb)
{
    tt_.resize(mb);
}

template <NodeType NODE_TYPE>
Score Searcher::search(int depth, SearchStack* ss, Score alpha, const Score beta, bool cutNode)
{
    constexpr bool IS_PV   = NODE_TYPE == NodeType::PV || NODE_TYPE == NodeType::ROOT;
    constexpr bool IS_ROOT = NODE_TYPE == NodeType::ROOT;

    // Cancel the search if required.
    if (stopSearch_.load() == true) {
        return 0;
    }

    TimeManager::Limit limits = timeManager_->check_node();

    if (limits != TimeManager::CONTINUE) {
        stop_searching();
        return 0;
    }

    // If we have no depth left, evaluate the node. We must resolve any loud moves or checks as we
    // cannot trust the static eval in such positions.
    if (depth <= 0) {
        return quiesce<IS_PV ? NodeType::PV : NodeType::NON_PV>(ss, alpha, beta);
    }

    if (hist_.pos().is_fifty_move() || hist_.pos().is_threefold()) {
        return DRAW_SCORE;
    }

    const Score originalAlpha = alpha;

    // Probe the transposition table.
    // If we have a value stored here that is higher quality than the search at the current depth,
    // we simply use this node as our eval.
    //
    // Otherwise, we use the best move from when this node was previously searched as a hint in
    // the move ordering.
    auto [ttMove, ttScore, ttDepth, ttTag, ttAge] = tt_.read(hist_.pos());
    if (ttTag != TtTag::EMPTY && ttDepth >= depth) {
        if constexpr (NODE_TYPE == NodeType::ROOT) {
            if (bestMove_.is_null()) {
                bestMove_ = ttMove;
                ss->pv.clear();
                ss->pv[0] = bestMove_;
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

    // We calculate the static eval and place it on the search stack. Placing it on the
    // search stack allows us to track improvement. Note that we cannot rely on evaluate() at all
    // if we are in check.
    ss->staticEval = hist_.pos().is_check() ? NO_SCORE : evaluate(hist_.pos());
    bool improving = is_improving(ss);

    bool inCheck = hist_.pos().is_check();

    // Pruning. It is incorrect to do so in check, or in a PV node.
    if (!inCheck && !IS_ROOT && !IS_PV) {
        // Reverse futility pruning. We prune if static eval is greater than beta plus some margin.

        // Making the margin more aggressive when we are improving gains ~20 ELO.
        const Score rfpMargin = 150 * depth - 75 * improving;

        // There are many parameters that can be adjusted here which may gain elo.
        if (ss->staticEval >= beta + rfpMargin) {
            return ss->staticEval;
        }

        // Null move pruning.
        // If the evaluation of passing a search is better than beta (plus some margin), a beta
        // cutoff is likely to occur so we can prune the node.
        const bool nmpIsOkNode = !(ss - 1)->isNull && !ss->isNmpVerification;

        if (nmpIsOkNode && depth >= 3 && ss->staticEval >= beta + 10 * depth) {
            constexpr int r = 4;
            add_move(Move{}, ss);
            Score nullMoveScore =
                -search<NodeType::NON_PV>(depth - r, ss + 1, -beta, -(beta - 1), !cutNode);
            pop_move(ss);

            if (nullMoveScore >= beta) {
                // We must verify the NMP value to avoid zugzwang. Here we stop NMP for a few plies
                // in order to not get stuck reverifying too often.
                int pliesToSkip = std::max(2, (depth - r) * 3 / 4);
                for (int i = 0; i < pliesToSkip; ++i) {
                    (ss + i)->isNmpVerification = true;
                }

                nullMoveScore = search<NodeType::NON_PV>(depth - r, ss, beta - 1, beta, false);

                for (int i = 0; i < pliesToSkip; ++i) {
                    (ss + i)->isNmpVerification = false;
                }

                if (nullMoveScore >= beta) {
                    return nullMoveScore;
                }
            }
        }

        // Razoring: if we are unlikely to raise alpha, we dive into a quiescent search and use that
        // score if it does not raise alpha.
        if (ss->staticEval + 150 * depth < alpha) {
            Score rScore = quiesce<IS_PV ? NodeType::PV : NodeType::NON_PV>(ss, alpha, alpha + 1);
            if (rScore < alpha) {
                return rScore;
            }
        }
    }

    auto  mp            = MovePicker::create(hist_.pos(), ttMove, ss, butterflyHistory_);
    int   searchedMoves = 0;
    Score bestScore     = NEG_INF;
    Move  bestMove      = Move{};

    StackVector<Move, MAX_LEGAL_MOVES> searchedQuiets{};
    bool                               skipQuiets = false;

    // Loop through all legal moves to search them.
    Move move;
    while (!(move = mp.next_move()).is_null()) {
        if (NODE_TYPE == NodeType::ROOT && stopSearch_.load()) {
            break;
        }

        bool isQuiet = !(move.is_promotion() || hist_.pos().is_capture(move));

        if (skipQuiets && isQuiet) {
            continue;
        }

        searchedMoves++;

        if (isQuiet) {
            searchedQuiets.push_back(move);
        }

        // Quiet move pruning.
        // In some situations it is extremely unlikely that a quiet move will improve eval.
        if (!IS_ROOT && !is_mate_score(bestScore)) {
            // LMP
            // If we are searching late and close to a leaf, we do not need to bother searching late
            // quiet moves.
            if (!hist_.pos().is_check() && searchedMoves >= 5 + 2 * depth * depth) {
                skipQuiets = true;
            }

            // Futility pruning.
            // If the static eval is such that only loud moves could improve alpha we stop bothering
            // with quiet moves.
            if (!hist_.pos().is_check() && depth <= 3 && ss->staticEval + 100 * depth <= alpha) {
                skipQuiets = true;
            }

            // If a move fails SEE, then we should prune it. We use different margins for quiets and
            // loud moves, mostly because other engines do so it seems reasonable to tune later on.
            Score seeMargin = isQuiet ? -70 * depth : -80 * depth;
            if (!hist_.pos().is_see_above(move, seeMargin)) {
                continue;
            }
        }

        // Make the move
        add_move(move, ss);

        bool givesCheck = hist_.pos().is_check();
        int  extensions = 0;

        // We want to avoid extensions in certain positions.
        if (!IS_ROOT && ss->ply < 2 * rootDepth_) {
            // If we are giving check then a position is worth looking into.
            if (givesCheck) {
                extensions = 1;
            }
        }

        // Late move reductions.
        // We are confident enough in our move ordering to say that late moves are likely not very
        // good, so we search them at a reduced depth. If we find the moves to be actually good
        // we must re-search.
        Score score = NEG_INF;
        if (depth >= 3 && searchedMoves > 1) {
            int r = 1 + std::log(depth) * std::log(searchedMoves) / 3;

            // Reducing reductions here appears to be required to make LMR gain.
            r -= hist_.pos().is_capture(move);

            // Reducing outside PV nodes appears to be a gainer.
            r += !IS_PV;

            // Reduce in expected cut nodes
            r += cutNode;

            r = std::clamp(r, 0, depth - 1);

            score = -search<NodeType::NON_PV>(depth - r - 1, ss + 1, -alpha - 1, -alpha, true);

            if (r >= 1 && score > alpha) {
                score = -search<NodeType::NON_PV>(depth + extensions - 1, ss + 1, -alpha - 1,
                                                  -alpha, !cutNode);
            }
        }
        else if (!IS_PV || searchedMoves != 1) {
            score = -search<NodeType::NON_PV>(depth + extensions - 1, ss + 1, -alpha - 1, -alpha,
                                              !cutNode);
        }

        // If we are in a PV node, and we are either searching the PV or a fail low, we must do a
        // full window search.
        if (IS_PV && (searchedMoves == 1 || score > alpha)) {
            score = -search<NodeType::PV>(depth + extensions - 1, ss + 1, -beta, -alpha, false);
        }

        pop_move(ss);

        // Do not update TT or bounds if we are out of time.
        if (stopSearch_.load()) {
            return 0;
        }

        // Handle an improved score.
        if (score > bestScore) {
            bestScore = score;
        }

        // Handle fail lows.
        if (score > alpha) {
            alpha = score;

            bestMove = move;

            if constexpr (IS_ROOT) {
                bestMove_ = move;
            }

            ss->pv.clear();
            ss->pv.push_back(move);
            for (const auto& m : (ss + 1)->pv) {
                if (m.is_null()) {
                    continue;
                }

                ss->pv.push_back(m);
            }
        }

        // Handle fail highs.
        if (score >= beta) {
            // If a quiet move if a fail high we should update quiet heuristics like history and
            // killers so it is favoured.
            if (isQuiet) {
                ss->killer = move;
                update_quiet_histories(depth, move, searchedQuiets);
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

    // Probe the transposition table.
    auto [ttMove, ttScore, ttDepth, ttTag, _] = tt_.read(hist_.pos());
    if (ttTag != TtTag::EMPTY) {
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

    // We use a "standing pat" as a lower bound. If we are in check, we use NEG_INF as a substitute
    // since we cannot trust eval.
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

    // If we are in check, we should also generate evasions (so all legal moves in the position).
    MovePicker mp; // default-construct first
    if (hist_.pos().is_check()) {
        mp = MovePicker::create(hist_.pos(), ttMove, ss, butterflyHistory_);
    }
    else {
        mp = MovePicker::create<MoveType::CAPTURES>(hist_.pos(), ttMove, ss, butterflyHistory_);
    }

    Move move;
    while (!(move = mp.next_move()).is_null()) {

        searchedMoves++;

        // If an exchange sequence is bad, then we don't bother searching the move.
        if (!hist_.pos().is_check() && !hist_.pos().is_see_above(move, -150)) {
            continue;
        }

        add_move(move, ss);

        Score score;
        // Search at a null window first .
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

        if (stopSearch_.load()) {
            return 0;
        }

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

    // Write to the TT.
    TtTag writeTag = TtTag::EXACT;
    if (bestScore <= originalAlpha) {
        writeTag = TtTag::UPPER;
    }
    else if (bestScore >= beta) {
        writeTag = TtTag::LOWER;
    }

    const Score writeScore = score_to_tt(bestScore, ss->ply);
    if (!stopSearch_.load()) {
        // We write to the TT at depth 0 since functionally a quiescence search node is the same as
        // a leaf node.
        tt_.write(hist_.pos(), bestMove, writeScore, 0, writeTag);
    }

    return bestScore;
}

void Searcher::add_move(const Move move, SearchStack* ss)
{
    hist_.add_move(move);
    ss->playedMove = move;
    ss->isNull     = move.is_null();
    (ss + 1)->pv.clear();

    reportData_.selDepth = std::max(reportData_.selDepth, (ss + 1)->ply);
}

void Searcher::pop_move(SearchStack* ss)
{
    hist_.pop_move();
    (void)ss;
}

void Searcher::update_quiet_histories(int depth, Move currMove,
                                      const StackVector<Move, MAX_LEGAL_MOVES>& searchedQuiets)
{
    Score bonus = history_bonus(depth);
    Score malus = history_malus(depth);

    update_butterfly_history(butterflyHistory_, hist_.pos(), currMove, bonus);

    for (Move move : searchedQuiets) {
        update_butterfly_history(butterflyHistory_, hist_.pos(), move, malus);
    }
}
} // namespace shellac
