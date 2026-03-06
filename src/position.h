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

#ifndef SHELLAC_POSITION_H
#define SHELLAC_POSITION_H
#include <array>
#include <optional>
#include <string>
#include <vector>

#include "bitboard.h"
#include "types.h"

namespace shellac {

class GameHistory;

class Position
{
public:
    static Position           from_fen(const std::string& fen);
    [[nodiscard]] std::string to_fen() const;

    Position(const Position& parent, Move move, const GameHistory& history);
    Position(const Position& parent, Move move);

    [[nodiscard]] Move parse_move(const std::string& move) const;

    [[nodiscard]] Color side_to_move() const;

    [[nodiscard]] std::optional<Square> en_passant() const;
    [[nodiscard]] bool                  can_castle_kingside(Color color) const;
    [[nodiscard]] bool                  can_castle_queenside(Color color) const;

    [[nodiscard]] constexpr Bitboard pieces() const;
    [[nodiscard]] constexpr Bitboard pieces(Color color) const;
    template <typename... PieceTypes,
              typename = std::enable_if_t<(std::is_same_v<PieceTypes, PieceType> && ...) &&
                                              (sizeof...(PieceTypes) > 0),
                                          Bitboard>>
    [[nodiscard]] constexpr Bitboard pieces(PieceTypes... pieceTypes) const;
    template <typename... PieceTypes,
              typename = std::enable_if_t<(std::is_same_v<PieceTypes, PieceType> && ...) &&
                                              (sizeof...(PieceTypes) > 0),
                                          Bitboard>>
    [[nodiscard]] constexpr Bitboard pieces(Color color, PieceTypes... pieceTypes) const;

    [[nodiscard]] Piece piece_at(Square square) const;

    [[nodiscard]] bool is_repetition_of(const Position& rhs) const;

private:
    enum CastlingRights : uint8_t
    {
        WHITE_KING  = 0b0001,
        WHITE_QUEEN = 0b0010,
        BLACK_KING  = 0b0100,
        BLACK_QUEEN = 0b1000,
    };

    Position() = default;

    void set_piece(Square at, Piece to);
    void swap_piece(Square at, Piece to);
    void remove_piece(Square at);
    void set_bitboard(Piece piece, Square at);
    void unset_bitboard(Piece piece, Square at);

    void set_castling(CastlingRights castling);
    void unset_castling(CastlingRights castling);
    void set_side_to_move(Color sideToMove);
    void set_en_passant(Square square);
    void clear_en_passant();
    void clear_castling_rights(CastlingRights castling);

    void apply_move(Move move);

    std::array<Piece, 64>   mailBox_{};
    std::uint8_t            castlingRights_{};
    Color                   sideToMove_{};
    Square                  enPassantSquare_{Square::INVALID};
    std::array<Bitboard, 6> pieceTypeBitboard_{};
    std::array<Bitboard, 2> colorBitboard_{};
    int                     fiftyMoveRule_{};
    int                     repetitions_{};
};

constexpr Bitboard Position::pieces() const
{
    return pieces(Color::WHITE) | pieces(Color::BLACK);
}

constexpr Bitboard Position::pieces(const Color color) const
{
    return colorBitboard_[underlying(color)];
}

template <typename... PieceTypes, typename>
constexpr Bitboard Position::pieces(PieceTypes... pieceTypes) const
{
    constexpr auto to_index = [](const PieceType pieceType)
    { return underlying(pieceType) - underlying(PieceType::PAWN); };

    return (pieceTypeBitboard_[to_index(pieceTypes)] | ...);
}

template <typename... PieceTypes, typename>
constexpr Bitboard Position::pieces(const Color color, PieceTypes... pieceTypes) const
{
    return pieces(color) & pieces(pieceTypes...);
}

class GameHistory
{
public:
    GameHistory(const std::string& fen, const std::vector<std::string>& moves);

    void add_move(const std::string& move);
    void add_move(Move move);
    void pop_move();

    [[nodiscard]] const Position& current_position() const;

    [[nodiscard]] auto begin()
    {
        return gameHistory_.begin();
    }

    [[nodiscard]] auto end()
    {
        return gameHistory_.end();
    }

    [[nodiscard]] auto begin() const
    {
        return gameHistory_.begin();
    }

    [[nodiscard]] auto end() const
    {
        return gameHistory_.end();
    }

    [[nodiscard]] auto cbegin() const
    {
        return gameHistory_.cbegin();
    }

    [[nodiscard]] auto cend() const
    {
        return gameHistory_.cend();
    }

    [[nodiscard]] auto rbegin() const
    {
        return gameHistory_.rbegin();
    }

    [[nodiscard]] auto rend() const
    {
        return gameHistory_.rend();
    }

    [[nodiscard]] auto rbegin()
    {
        return gameHistory_.rbegin();
    }

    [[nodiscard]] auto rend()
    {
        return gameHistory_.rend();
    }

private:
    std::vector<Position> gameHistory_;
};

} // namespace shellac

#endif // SHELLAC_POSITION_H
