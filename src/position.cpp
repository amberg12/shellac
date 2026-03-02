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

#include "position.h"

#include <sstream>

namespace shellac {

Position Position::from_fen(const std::string& fen)
{
    std::string        token;
    std::istringstream iss(fen);
    Position           out{};

    iss >> token;
    Rank rank = Rank::R_8;
    File file = File::F_A;
    for (const char c : token) {
        if (c == '/') {
            --rank;
            file = File::F_A;
            continue;
        }

        if (std::isdigit(c)) {
            file += c - '0';
        }
        else {
            out.add_piece(make_square(file, rank), from_char<Piece>(c));
            ++file;
        }
    }

    iss >> token;
    out.set_side_to_move(from_char<Color>(token[0]));

    iss >> token;
    for (const char c : token) {
        if (c == 'K') {
            out.set_castling(WHITE_KING);
        }

        if (c == 'Q') {
            out.set_castling(WHITE_QUEEN);
        }

        if (c == 'k') {
            out.set_castling(BLACK_KING);
        }

        if (c == 'q') {
            out.set_castling(BLACK_QUEEN);
        }
    }

    iss >> token;
    Square enPassant = from_string<Square>(token);
    out.set_en_passant(enPassant);

    if (iss >> token) {
        out.fiftyMoveRule_ = std::stoi(token);
    }
    else {
        out.fiftyMoveRule_ = 0;
    }

    return out;
}

std::string Position::to_fen() const
{
    std::ostringstream out;

    for (Rank rank = Rank::R_8; is_valid(rank); --rank) {
        int empty = 0;

        for (File file = File::F_A; is_valid(file); ++file) {
            Square sq   = make_square(file, rank);
            Piece  piece = piece_at(sq);

            if (piece == Piece::NONE) {  // adjust if your empty value differs
                ++empty;
            } else {
                if (empty > 0) {
                    out << empty;
                    empty = 0;
                }
                out << to_char(piece);  // your existing helper
            }
        }

        if (empty > 0)
            out << empty;

        if (rank != Rank::R_1)
            out << '/';
    }

    out << ' ';
    out << (sideToMove_ == Color::WHITE ? 'w' : 'b');

    out << ' ';
    if (castlingRights_ == 0) {
        out << '-';
    } else {
        if (castlingRights_ & WHITE_KING)  out << 'K';
        if (castlingRights_ & WHITE_QUEEN) out << 'Q';
        if (castlingRights_ & BLACK_KING)  out << 'k';
        if (castlingRights_ & BLACK_QUEEN) out << 'q';
    }

    // 4. En passant
    out << ' ';
    if (enPassantSquare_ == Square::INVALID) {
        out << '-';
    } else {
        out << to_char(file_of(enPassantSquare_))
            << to_char(rank_of(enPassantSquare_));
    }

    out << ' ' << fiftyMoveRule_;
    out << ' ' << 1;

    return out.str();
}

Piece Position::piece_at(const Square square) const
{
    return mailBox_[underlying(square)];
}

void Position::add_piece(const Square at, const Piece piece)
{
    mailBox_[underlying(at)] = piece;
}

void Position::set_castling(const CastlingRights castling)
{
    castlingRights_ |= castling;
}

void Position::set_side_to_move(const Color sideToMove)
{
    sideToMove_ = sideToMove;
}

void Position::set_en_passant(const Square square)
{
    enPassantSquare_ = square;
}

} // namespace shellac
