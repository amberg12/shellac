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

#include <iostream>
#include <sstream>
#include <string>

#include "uci.h"

#include <vector>

#include "types.h"

namespace shellac {
void UciEngine::loop()
{
    std::string token, line;
    while (token != "quit") {
        std::getline(std::cin, line);
        std::istringstream iss{line};

        iss >> token;

        if (token == "position") {
            std::string              fen;
            std::vector<std::string> moves;
            iss >> token;

            if (token == "startpos") {
                fen = STARTING_POSITION;
            }
            else {
                while (iss >> token && token != "moves") {
                    fen += token + " ";
                }
            }

            iss >> token;

            while (iss >> token) {
                moves.push_back(token);
            }

            engine_.set_position(fen, moves);
        }
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "uci") {
            std::cout << "id name Shellac " << BuildIdentifier << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (token == "go") {
            engine_.go(SearchLimits{});
        }
        else if (token == "d") {
            std::cout << engine_.display();
        }
        else if (token == "perft") {
            int depth;
            iss >> depth;
            engine_.perft(depth);
        }
        else if (token == "perft_suite") {
            int         depth = 0;
            std::string path;
            iss >> depth;
            iss >> path;
            engine_.perft_suite(depth, path);
        }

        std::cout << std::flush;
    }
}
} // namespace shellac
