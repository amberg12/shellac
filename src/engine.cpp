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

namespace shellac {
void Engine::set_position(const std::string& fen, const std::vector<std::string>& moves)
{
    (void)moves;
    position_ = Position::from_fen(fen);
}

std::string Engine::display() const
{
    std::ostringstream out;

    out << "+-+-+-+-+-+-+-+-+" << '\n';
    for (Rank rank = Rank::R_8; is_valid(rank); --rank) {
        for (File file = File::F_A; is_valid(file); ++file) {
            const Square at    = make_square(file, rank);
            const Piece  piece = position_.piece_at(at);
            out << "|" << to_char(piece);
        }
        out << "|" << to_char(rank) << '\n';
        out << "+-+-+-+-+-+-+-+-+" << '\n';
    }
    out << " a b c d e f g h " << '\n';

    out << "Fen: " + position_.to_fen();

    return out.str();
}
} // namespace shellac
