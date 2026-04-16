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

#include "movegen.h"

#include "position.h"

namespace shellac {
namespace {

template <MoveType MOVE_TYPE>
Move* generate_pawn_moves(Move* begin, const Position& position)
{
    const Bitboard enPassantMask = [&]()
    {
        if (position.en_passant().has_value()) {
            return Bitboard(position.en_passant().value());
        }

        return Bitboard{};
    }();

    for (const Square src : position.pieces(position.side_to_move(), PieceType::PAWN)) {
        const Bitboard blockers = position.pieces() | enPassantMask;
        const Bitboard destinations =
            generate_pawn_destinations(position.side_to_move(), src, blockers);
        const Bitboard validDestinations = [&]
        {
            if constexpr (MOVE_TYPE == MoveType::kCaptures) {
                return destinations & position.pieces(~position.side_to_move());
            }

            return destinations & ~position.pieces(position.side_to_move());
        }();

        for (const Square dst : validDestinations) {
            if (!(Bitboard{dst} & enPassantMask).is_empty()) {
                *begin++ = Move::create_en_passant(src, dst);
            }
            else if (rank_of(dst) == Rank::R_1 || rank_of(dst) == Rank::R_8) {
                *begin++ = Move::create_promotion(src, dst, PieceType::QUEEN);
                *begin++ = Move::create_promotion(src, dst, PieceType::ROOK);
                *begin++ = Move::create_promotion(src, dst, PieceType::BISHOP);
                *begin++ = Move::create_promotion(src, dst, PieceType::KNIGHT);
            }
            else {
                *begin++ = Move{src, dst};
            }
        }
    }

    return begin;
}

template <MoveType MOVE_TYPE, PieceType PIECE_TYPE>
Move* generate_moves_for(Move* begin, const Position& position)
{
    for (const Square src : position.pieces(position.side_to_move(), PIECE_TYPE)) {
        const Bitboard attacks      = generate_attacks<PIECE_TYPE>(src, position.pieces());
        const Bitboard destinations = [&]
        {
            if constexpr (MOVE_TYPE == MoveType::kCaptures) {
                return attacks & position.pieces(~position.side_to_move());
            }

            return attacks & ~position.pieces(position.side_to_move());
        }();

        for (const Square dst : destinations) {
            *begin++ = Move(src, dst);
        }
    }

    return begin;
}

template <MoveType MOVE_TYPE>
Move* generate_king_moves(Move* begin, const Position& position)
{
    const Square   src          = position.king_square(position.side_to_move());
    const Bitboard attacks      = generate_attacks<PieceType::KING>(src, position.pieces());
    const Bitboard destinations = [&]
    {
        if constexpr (MOVE_TYPE == MoveType::kCaptures) {
            return attacks & position.pieces(~position.side_to_move());
        }
        return attacks & ~position.pieces(position.side_to_move());
    }();

    for (const Square dst : destinations) {
        *begin++ = Move(src, dst);
    }

    if constexpr (MOVE_TYPE == MoveType::kCaptures) {
        return begin;
    }

    if (position.side_to_move() == Color::WHITE && src == Square::E1) {
        if (position.can_castle_kingside(Color::WHITE) &&
            position.piece_at(Square::H1) == Piece::W_ROOK &&
            position.piece_at(Square::F1) == Piece::NONE &&
            position.piece_at(Square::G1) == Piece::NONE) {
            *begin++ = Move::create_castle(src, Square::G1);
        }

        if (position.can_castle_queenside(Color::WHITE) &&
            position.piece_at(Square::A1) == Piece::W_ROOK &&
            position.piece_at(Square::D1) == Piece::NONE &&
            position.piece_at(Square::C1) == Piece::NONE &&
            position.piece_at(Square::B1) == Piece::NONE) {
            *begin++ = Move::create_castle(src, Square::C1);
        }
    }
    else if (position.side_to_move() == Color::BLACK && src == Square::E8) {
        if (position.can_castle_kingside(Color::BLACK) &&
            position.piece_at(Square::H8) == Piece::B_ROOK &&
            position.piece_at(Square::F8) == Piece::NONE &&
            position.piece_at(Square::G8) == Piece::NONE) {
            *begin++ = Move::create_castle(src, Square::G8);
        }

        if (position.can_castle_queenside(Color::BLACK) &&
            position.piece_at(Square::A8) == Piece::B_ROOK &&
            position.piece_at(Square::D8) == Piece::NONE &&
            position.piece_at(Square::C8) == Piece::NONE &&
            position.piece_at(Square::B8) == Piece::NONE) {
            *begin++ = Move::create_castle(src, Square::C8);
        }
    }

    return begin;
}
} // namespace

template <MoveType MOVE_TYPE>
Move* generate_moves(Move* begin, const Position& position)
{
    begin = generate_pawn_moves<MOVE_TYPE>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::KNIGHT>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::BISHOP>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::ROOK>(begin, position);
    begin = generate_moves_for<MOVE_TYPE, PieceType::QUEEN>(begin, position);
    begin = generate_king_moves<MOVE_TYPE>(begin, position);
    return begin;
}

template Move* generate_moves<MoveType::kNormal>(Move* begin, const Position& position);
template Move* generate_moves<MoveType::kCaptures>(Move* begin, const Position& position);

} // namespace shellac
