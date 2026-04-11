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
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "uci.h"

#include <algorithm>

#include "types.h"

namespace shellac {
namespace {
bool is_go_keyword(const std::string& token)
{
    return token == "searchmoves" || token == "ponder" || token == "wtime" || token == "btime" ||
        token == "winc" || token == "binc" || token == "movestogo" || token == "depth" ||
        token == "nodes" || token == "mate" || token == "movetime" || token == "infinite";
}

std::optional<int> parse_int_token(const std::string& token)
{
    int                value = 0;
    std::istringstream parser{token};

    if ((parser >> value) && parser.eof()) {
        return value;
    }

    return std::nullopt;
}
} // namespace

void UciEngine::loop()
{
    std::string              token, line;
    std::string              currentFen = STARTING_POSITION;
    std::vector<std::string> currentMoves;

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
                if (iss >> token && token != "moves") {
                    token.clear();
                }
            }
            else {
                if (token != "fen") {
                    fen += token + " ";
                }
                while (iss >> token && token != "moves") {
                    fen += token + " ";
                }
            }

            if (token == "moves") {
                while (iss >> token) {
                    moves.push_back(token);
                }
            }

            currentFen   = fen;
            currentMoves = moves;
            engine_.set_position(currentFen, currentMoves);
        }
        else if (token == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (token == "stop") {
            engine_.stop();
        }
        else if (token == "uci") {
            std::cout << "id name Shellac " << BuildIdentifier << std::endl;
            std::cout << "option name Hash type spin default 16 min 1 max 131072" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (token == "go") {
            SearchLimits             limits{};
            std::vector<std::string> args;

            while (iss >> token) {
                args.push_back(token);
            }

            for (size_t i = 0; i < args.size(); ++i) {
                const std::string& arg = args[i];

                const auto parse_optional_int = [&](std::optional<int>& field)
                {
                    if (i + 1 >= args.size()) {
                        return;
                    }

                    if (const auto parsed = parse_int_token(args[i + 1]); parsed.has_value()) {
                        field = *parsed;
                        ++i;
                    }
                };

                if (arg == "searchmoves") {
                    std::vector<Move> allowedMoves;
                    const GameHistory history{currentFen, currentMoves};

                    while (i + 1 < args.size() && !is_go_keyword(args[i + 1])) {
                        const std::string& moveString = args[++i];
                        allowedMoves.push_back(history.pos().parse_move(moveString));
                    }

                    if (!allowedMoves.empty()) {
                        limits.searchMoves = allowedMoves;
                    }
                }
                else if (arg == "wtime") {
                    parse_optional_int(limits.whiteTime);
                }
                else if (arg == "btime") {
                    parse_optional_int(limits.blackTime);
                }
                else if (arg == "winc") {
                    parse_optional_int(limits.whiteInc);
                }
                else if (arg == "binc") {
                    parse_optional_int(limits.blackInc);
                }
                else if (arg == "movestogo") {
                    parse_optional_int(limits.movesToGo);
                }
                else if (arg == "depth") {
                    parse_optional_int(limits.depth);
                }
                else if (arg == "nodes") {
                    parse_optional_int(limits.nodes);
                }
                else if (arg == "mate") {
                    parse_optional_int(limits.mate);
                }
                else if (arg == "movetime") {
                    parse_optional_int(limits.moveTime);
                }
                else if (arg == "infinite") {
                    limits.infinite = true;
                }
            }

            engine_.go(limits);
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
        else if (token == "ucinewgame") {
            engine_.new_game();
        }
        else if (token == "setoption") {
            std::string name, value, word;

            if (iss >> word && word == "name") {
                while (iss >> word && word != "value") {
                    if (!name.empty())
                        name += " ";
                    name += word;
                }
            }

            if (word == "value") {
                while (iss >> word) {
                    if (!value.empty())
                        value += " ";
                    value += word;
                }
            }

            if (name == "Hash") {
                if (auto parsed = parse_int_token(value); parsed.has_value()) {
                    size_t mb = std::clamp(*parsed, 1, 131072);
                    engine_.set_hash_size(mb);
                }
            }
        }
    }

    std::cout << std::flush;
}
} // namespace shellac
