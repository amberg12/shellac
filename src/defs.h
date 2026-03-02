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

#ifndef SHELLAC_CHESS_DEFS_H
#define SHELLAC_CHESS_DEFS_H

namespace shellac {
#ifdef BUILD_IDENTIFIER
constexpr const char* BuildIdentifier = BUILD_IDENTIFIER;
#else
constexpr const char* BuildIdentifier = "unknown-build";
#endif // BUILD_IDENTIFIER
} // namespace shellac

#endif // SHELLAC_CHESS_DEFS_H
