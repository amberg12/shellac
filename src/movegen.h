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
// Created by amber on 04/03/2026.
//

#ifndef SHELLAC_MOVEGEN_H
#define SHELLAC_MOVEGEN_H

#include <cstring>

#include "types.h"

namespace shellac {
static constexpr usize kMaxLegalMoves = 320;

class Position;

enum class MoveType
{
    kNormal,
    kCaptures,
};

template <MoveType MOVE_TYPE>
Move* generate_moves(Move* begin, const Position& position);

} // namespace shellac

#endif // SHELLAC_MOVEGEN_H
