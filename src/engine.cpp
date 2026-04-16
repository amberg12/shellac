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

#include "engine.h"

#include <atomic>
#include <fstream>
#include <mutex>
#include <sstream>

#include "movegen.h"
#include "movepicker.h"

namespace shellac {
namespace {
template <bool OUTPUT_MOVES>
uint64_t run_perft(const Position& position, const int depth)
{
    if (depth == 0) {
        return 1;
    }

    uint64_t sum = 0;

    std::array<Move, kMaxLegalMoves> moveList{};
    Move* end            = generate_moves<MoveType::kNormal>(moveList.data(), position);
    usize movesGenerated = end - moveList.data();

    for (usize i = 0; i < movesGenerated; ++i) {
        Move move = moveList[i];

        if (!position.is_legal(move)) {
            continue;
        }

        const auto     next       = Position{position, move};
        const uint64_t move_count = run_perft<false>(next, depth - 1);

        if constexpr (OUTPUT_MOVES) {
            std::cout << move << ": " << move_count << '\n';
        }

        sum += move_count;
    }

    return sum;
}
} // namespace

void Engine::set_position(const std::string& fen, const std::vector<std::string>& moves)
{
    gameHistory_ = GameHistory{fen, moves};
}

void Engine::go(const SearchLimits& searchLimits)
{
    auto run_search = [this, searchLimits]()
    {
        const SearchLimits limits      = searchLimits;
        const GameHistory  gameHistory = this->gameHistory_;
        std::lock_guard    guard{searcherMutex_};
        searcher_->begin_search(gameHistory, limits);
    };

    threadPool_.enqueue(run_search);
}

void Engine::stop()
{
    searcher_->stop_searching();
}

void Engine::new_game()
{
    searcher_->new_game();
}

void Engine::set_hash_size(size_t hash_size)
{
    searcher_->set_hash_size(hash_size);
}

std::string Engine::display() const
{
    std::ostringstream out;

    out << "+-+-+-+-+-+-+-+-+" << '\n';
    for (Rank rank = Rank::R_8; is_valid(rank); --rank) {
        for (File file = File::F_A; is_valid(file); ++file) {
            const Square at    = make_square(file, rank);
            const Piece  piece = gameHistory_.pos().piece_at(at);
            out << "|" << to_char(piece);
        }
        out << "|" << to_char(rank) << '\n';
        out << "+-+-+-+-+-+-+-+-+" << '\n';
    }
    out << " a b c d e f g h " << '\n';

    out << "Fen: " + gameHistory_.pos().to_fen() << '\n';

    return out.str();
}

void Engine::perft(const int depth) const
{
    const uint64_t nodes = run_perft<true>(gameHistory_.pos(), depth);
    std::cout << "Node count: " << nodes << '\n';
}

void Engine::perft_suite(const int maxDepth, const std::string& path)
{
    std::mutex ioMutex;

    const auto run_line = [](const int maxDepth, const std::string& line)
    {
        std::stringstream ss(line);

        std::string fen;
        std::getline(ss, fen, ';');

        Position position = Position::from_fen(fen);

        std::string   token;
        std::uint64_t expected = 0;
        bool          found    = false;

        while (std::getline(ss, token, ';')) {
            std::stringstream tok(token);

            char d;
            int  depth;

            tok >> d >> depth >> expected;

            if (depth == maxDepth) {
                found = true;
                break;
            }
        }

        if (!found) {
            return;
        }

        const uint64_t result = run_perft<false>(position, maxDepth);

        if (result != expected) {
            std::cout << position.to_fen() << '\n'
                      << "Depth " << maxDepth << " FAIL expected " << expected << " got " << result
                      << '\n';
        }
    };

    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open: " << path << '\n';
        return;
    }

    std::string                    line;
    std::vector<std::string>       lines;
    std::vector<std::future<void>> futures;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        lines.push_back(line);
    }

    const int       futureSum = static_cast<int>(lines.size());
    std::atomic_int ranTests{0};

    for (const auto& suiteLine : lines) {
        futures.push_back(threadPool_.enqueue(
            [&, suiteLine]
            {
                run_line(maxDepth, suiteLine);
                const int done = ranTests.fetch_add(1, std::memory_order_relaxed) + 1;
                const int step = std::max(1, futureSum / 20);

                if (done % step == 0 || done == futureSum) {
                    std::lock_guard lock(ioMutex);
                    std::cout << done << '/' << futureSum << '\n' << std::flush;
                }
            }));
    }

    for (auto& future : futures) {
        future.get();
    }

    std::cout << "ran all\n";
}
} // namespace shellac
