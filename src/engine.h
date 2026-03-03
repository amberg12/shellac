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

#ifndef SHELLAC_ENGINE_H
#define SHELLAC_ENGINE_H
#include <vector>

#include "position.h"

namespace shellac {

class Engine
{
public:
    void set_position(const std::string& fen, const std::vector<std::string>& moves);

    [[nodiscard]] std::string display() const;
private:
    GameHistory gameHistory_{std::string(STARTING_POSITION), {}};
};

} // namespace shellac

#endif // SHELLAC_ENGINE_H
