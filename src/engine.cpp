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

#include "engine.h"

#include <sstream>

#include "movegen.h"

namespace shellac {
namespace {
template <bool OUTPUT_MOVES>
uint64_t run_perft(const Position& position, const int depth)
{
    if (!OUTPUT_MOVES && depth == 1) {
        const MoveList moves = MoveList::from_position(position);
        return moves.size();
    }

    if (depth == 0) {
        return 1;
    }

    uint64_t       sum   = 0;
    const MoveList moves = MoveList::from_position(position);

    for (const Move& move : moves) {
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

std::string Engine::display() const
{
    std::ostringstream out;

    out << "+-+-+-+-+-+-+-+-+" << '\n';
    for (Rank rank = Rank::R_8; is_valid(rank); --rank) {
        for (File file = File::F_A; is_valid(file); ++file) {
            const Square at    = make_square(file, rank);
            const Piece  piece = gameHistory_.current_position().piece_at(at);
            out << "|" << to_char(piece);
        }
        out << "|" << to_char(rank) << '\n';
        out << "+-+-+-+-+-+-+-+-+" << '\n';
    }
    out << " a b c d e f g h " << '\n';

    out << "Fen: " + gameHistory_.current_position().to_fen() << '\n';

    return out.str();
}

void Engine::perft(const int depth) const
{
    const uint64_t nodes = run_perft<true>(gameHistory_.current_position(), depth);
    std::cout << "Node count: " << nodes << '\n';
}
} // namespace shellac
