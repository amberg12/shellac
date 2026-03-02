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

#ifndef SHELLAC_POSITION_H
#define SHELLAC_POSITION_H
#include <array>
#include <string>

#include "types.h"

namespace shellac {

class Position
{
public:
    static Position           from_fen(const std::string& fen);
    [[nodiscard]] std::string to_fen() const;

    [[nodiscard]] Piece piece_at(Square square) const;
private:
    enum CastlingRights : uint8_t
    {
        WHITE_KING  = 0b0001,
        WHITE_QUEEN = 0b0010,
        BLACK_KING  = 0b0100,
        BLACK_QUEEN = 0b1000,
    };

    void add_piece(Square at, Piece piece);
    void set_castling(CastlingRights castling);
    void set_side_to_move(Color sideToMove);
    void set_en_passant(Square square);

    std::array<Piece, 64> mailBox_{};
    std::uint8_t          castlingRights_{};
    Color                 sideToMove_{};
    Square                enPassantSquare_{Square::INVALID};
    int                   fiftyMoveRule_{};
};

} // namespace shellac

#endif // SHELLAC_POSITION_H
