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

#include <algorithm>
#include <cstring>
#include <random>
#include <sstream>

namespace shellac {
namespace {
namespace z {
std::uint64_t PieceHash[32][64];
std::uint64_t CastleHash[16];
std::uint64_t EnPassantHash[8];
std::uint64_t SideToMoveHash;
} // namespace z

size_t piece_to_piece_type_index(const Piece piece)
{
    return underlying(type_of(piece)) - 1;
}

size_t piece_to_color_index(const Piece piece)
{
    return underlying(color_of(piece));
}
} // namespace

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
            const Piece  to = from_char<Piece>(c);
            const Square at = make_square(file, rank);
            out.set_piece(at, to);
            ++file;
        }
    }

    iss >> token;
    out.set_side_to_move(from_char<Color>(token[0]));
    if (out.side_to_move() == Color::BLACK) {
        out.hash_ ^= z::SideToMoveHash;
    }

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

    out.generate_check_info();

    return out;
}

std::string Position::to_fen() const
{
    std::ostringstream out;

    for (Rank rank = Rank::R_8; is_valid(rank); --rank) {
        int empty = 0;

        for (File file = File::F_A; is_valid(file); ++file) {
            Square sq    = make_square(file, rank);
            Piece  piece = piece_at(sq);

            if (piece == Piece::NONE) { // adjust if your empty value differs
                ++empty;
            }
            else {
                if (empty > 0) {
                    out << empty;
                    empty = 0;
                }
                out << to_char(piece); // your existing helper
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
    }
    else {
        if (castlingRights_ & WHITE_KING)
            out << 'K';
        if (castlingRights_ & WHITE_QUEEN)
            out << 'Q';
        if (castlingRights_ & BLACK_KING)
            out << 'k';
        if (castlingRights_ & BLACK_QUEEN)
            out << 'q';
    }

    // 4. En passant
    out << ' ';
    if (enPassantSquare_ == Square::INVALID) {
        out << '-';
    }
    else {
        out << to_char(file_of(enPassantSquare_)) << to_char(rank_of(enPassantSquare_));
    }

    out << ' ' << fiftyMoveRule_;
    out << ' ' << 1;

    return out.str();
}

Position::Position(const Position& parent, const Move move) :
    mailBox_{parent.mailBox_}, castlingRights_{parent.castlingRights_},
    sideToMove_{parent.sideToMove_}, enPassantSquare_{parent.enPassantSquare_},
    pieceTypeBitboard_{parent.pieceTypeBitboard_}, colorBitboard_{parent.colorBitboard_},
    fiftyMoveRule_{parent.fiftyMoveRule_ + 1}, hash_{parent.hash_}
{
    apply_move(move);
    generate_check_info();
}

Position::Position(const Position& parent, const Move move, const GameHistory& history) :
    mailBox_{parent.mailBox_}, castlingRights_{parent.castlingRights_},
    sideToMove_{parent.sideToMove_}, enPassantSquare_{parent.enPassantSquare_},
    pieceTypeBitboard_{parent.pieceTypeBitboard_}, colorBitboard_{parent.colorBitboard_},
    fiftyMoveRule_{parent.fiftyMoveRule_ + 1}, hash_{parent.hash_}
{
    apply_move(move);
    generate_check_info();
    if (fiftyMoveRule_ != 0 && !move.is_castle()) {
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            const auto& [pos, prevMove] = *it;
            if (is_repetition_of(pos)) {
                repetitions_ = pos.repetitions_ + 1;
                break;
            }

            if (pos.fiftyMoveRule_ == 0) {
                break;
            }
        }
    }
}

Move Position::parse_move(const std::string& move) const
{
    assert(move.size() == 4 || move.size() == 5);

    const Square    src         = from_string<Square>(move.substr(0, 2));
    const Square    dst         = from_string<Square>(move.substr(2, 2));
    const PieceType movingPiece = type_of(piece_at(src));

    if (move.size() == 5) {
        const PieceType promoteTo = type_of(from_char<Piece>(move[4]));
        return Move::create_promotion(src, dst, promoteTo);
    }

    if (movingPiece == PieceType::PAWN && dst == enPassantSquare_) {
        return Move::create_en_passant(src, dst);
    }

    const int diff =
        std::abs(static_cast<int>(underlying(src)) - static_cast<int>(underlying(dst)));
    if (movingPiece == PieceType::KING && diff == 2) {
        return Move::create_castle(src, dst);
    }

    return {src, dst};
}

bool Position::is_legal(const Move move) const
{
    const Color     us          = side_to_move();
    const Color     them        = ~us;
    const Square    src         = move.src();
    const Square    dst         = move.dst();
    const PieceType movingPiece = type_of(piece_at(src));

    if (kingAttackers_ >= 2) {
        return movingPiece == PieceType::KING && !move.is_castle() &&
            !attackedSquares_.has_square(dst);
    }

    if (move.is_en_passant()) {
        const Direction direction =
            sideToMove_ == Color::WHITE ? Direction::SOUTH : Direction::NORTH;
        const Bitboard appliedBoard =
            pieces() ^ Bitboard(enPassantSquare_) ^ Bitboard(src) ^ Bitboard(dst + direction);

        const Bitboard rookAttacks =
            generate_attacks<PieceType::ROOK>(king_square(us), appliedBoard);

        if (rookAttacks.intersects(pieces(them, PieceType::ROOK, PieceType::QUEEN))) {
            return false;
        }

        const Bitboard bishopAttacks =
            generate_attacks<PieceType::BISHOP>(king_square(us), appliedBoard);

        if (bishopAttacks.intersects(pieces(them, PieceType::BISHOP, PieceType::QUEEN))) {
            return false;
        }

        if (blockers_.has_square(enPassantSquare_ + direction)) {
            return true;
        }
    }

    if (move.is_castle()) {
        if (kingAttackers_ != 0) {
            return false;
        }

        if (dst == Square::C1) {
            return !(attackedSquares_.has_square(Square::C1) ||
                     attackedSquares_.has_square(Square::D1));
        }

        if (dst == Square::G1) {
            return !(attackedSquares_.has_square(Square::F1) ||
                     attackedSquares_.has_square(Square::G1));
        }

        if (dst == Square::C8) {
            return !(attackedSquares_.has_square(Square::C8) ||
                     attackedSquares_.has_square(Square::D8));
        }

        if (dst == Square::G8) {
            return !(attackedSquares_.has_square(Square::F8) ||
                     attackedSquares_.has_square(Square::G8));
        }
    }

    if (movingPiece == PieceType::KING) {
        return !attackedSquares_.has_square(dst);
    }

    if (kingAttackers_ == 1) {
        if (!blockers_.has_square(dst)) {
            return false;
        }
    }

    if (pinRays_.has_square(src)) {
        if (movingPiece == PieceType::KNIGHT) {
            return false;
        }

        const Square ourKingSquare = king_square(sideToMove_);
        const bool   alignedWithKing =
            is_diagonal_to(dst, ourKingSquare) || is_orthogonal_to(dst, ourKingSquare);

        if (!alignedWithKing) {
            return false;
        }

        const Bitboard kingToSrc = Bitboard::generate_line(ourKingSquare, src);
        const Bitboard kingToDst = Bitboard::generate_line(ourKingSquare, dst);

        return kingToSrc.has_square(dst) || kingToDst.has_square(src);
    }

    return true;
}

bool Position::is_capture(const Move move) const
{
    return move.is_en_passant() || piece_at(move.dst()) != Piece::NONE;
}

Color Position::side_to_move() const
{
    return sideToMove_;
}

std::optional<Square> Position::en_passant() const
{
    if (!is_valid(enPassantSquare_)) {
        return std::nullopt;
    }
    else {
        return enPassantSquare_;
    }
}

bool Position::can_castle_kingside(const Color color) const
{
    const auto castlingRight = static_cast<CastlingRights>(
        color == Color::WHITE ? CastlingRights::WHITE_KING : CastlingRights::BLACK_KING);
    return castlingRights_ & castlingRight;
}

bool Position::can_castle_queenside(const Color color) const
{
    const auto castlingRight = static_cast<CastlingRights>(
        color == Color::WHITE ? CastlingRights::WHITE_QUEEN : CastlingRights::BLACK_QUEEN);
    return castlingRights_ & castlingRight;
}

Square Position::king_square(const Color color) const
{
    return pieces(color, PieceType::KING).lsb_square();
}

Piece Position::piece_at(const Square square) const
{
    return mailBox_[underlying(square)];
}

bool Position::is_threefold() const
{
    return repetitions_ >= 2;
}

bool Position::is_fifty_move() const
{
    return fiftyMoveRule_ >= 50;
}

bool Position::is_check() const
{
    return kingAttackers_ != 0;
}

bool Position::is_repetition_of(const Position& rhs) const
{
    const bool sideToMoveEq     = sideToMove_ == rhs.sideToMove_;
    const bool enPassantEq      = enPassantSquare_ == rhs.enPassantSquare_;
    const bool castlingRightsEq = castlingRights_ == rhs.castlingRights_;
    const bool piecesEq         = mailBox_ == rhs.mailBox_;

    return sideToMoveEq && enPassantEq && castlingRightsEq && piecesEq;
}

std::uint64_t Position::hash() const
{
    return hash_;
}

void Position::set_piece(const Square at, const Piece to)
{
    assert(piece_at(at) == Piece::NONE);
    hash_ ^= z::PieceHash[underlying(to)][underlying(at)];
    mailBox_[underlying(at)] = to;
    set_bitboard(to, at);
}

void Position::swap_piece(const Square at, const Piece to)
{
    assert(piece_at(at) != Piece::NONE);

    const Piece removedPiece = piece_at(at);
    unset_bitboard(piece_at(at), at);
    hash_ ^= z::PieceHash[underlying(removedPiece)][underlying(at)];
    hash_ ^= z::PieceHash[underlying(to)][underlying(at)];
    set_bitboard(to, at);
    mailBox_[underlying(at)] = to;
}

void Position::remove_piece(const Square at)
{
    assert(piece_at(at) != Piece::NONE);
    const Piece removedPiece = piece_at(at);
    hash_ ^= z::PieceHash[underlying(removedPiece)][underlying(at)];
    unset_bitboard(removedPiece, at);
    mailBox_[underlying(at)] = Piece::NONE;
}

void Position::set_bitboard(Piece piece, Square at)
{
    const size_t colorIndex     = piece_to_color_index(piece);
    const size_t pieceTypeIndex = piece_to_piece_type_index(piece);

    colorBitboard_[colorIndex].set_square(at);
    pieceTypeBitboard_[pieceTypeIndex].set_square(at);
}
void Position::unset_bitboard(Piece piece, Square at)
{
    const size_t colorIndex     = piece_to_color_index(piece);
    const size_t pieceTypeIndex = piece_to_piece_type_index(piece);

    colorBitboard_[colorIndex].unset_square(at);
    pieceTypeBitboard_[pieceTypeIndex].unset_square(at);
}

void Position::set_castling(const CastlingRights castling)
{
    const std::uint8_t oldRights = castlingRights_;
    castlingRights_ |= castling;
    hash_ ^= z::CastleHash[oldRights] ^ z::CastleHash[castlingRights_];
}

void Position::unset_castling(const CastlingRights castling)
{
    const auto oldRights = castlingRights_;
    castlingRights_ &= ~castling;
    hash_ ^= z::CastleHash[oldRights] ^ z::CastleHash[castlingRights_];
}

void Position::set_side_to_move(const Color sideToMove)
{
    sideToMove_ = sideToMove;
}

void Position::set_en_passant(const Square square)
{
    assert(enPassantSquare_ == Square::INVALID);
    hash_ ^= z::EnPassantHash[underlying(file_of(square))];
    enPassantSquare_ = square;
}

void Position::clear_en_passant()
{
    if (enPassantSquare_ != Square::INVALID) {
        const int hashIndex = underlying(file_of(enPassantSquare_));
        hash_ ^= z::EnPassantHash[hashIndex];
        enPassantSquare_ = Square::INVALID;
    }
}

void Position::clear_castling_rights(const CastlingRights castling)
{
    castlingRights_ &= ~castling;
}

void Position::apply_move(Move move)
{
    const Color  us   = sideToMove_;
    const Color  them = ~us;
    const Square src  = move.src();
    const Square dst  = move.dst();

    hash_ ^= z::SideToMoveHash;

    // We want to allow a 'null' move to be taken where we simply pass our turn.
    if (move.is_null()) {
        set_side_to_move(them);
        return;
    }

    const auto get_captured_piece = [&](const Square d)
    {
        if (move.is_en_passant())
            return make_piece(them, PieceType::PAWN);
        return piece_at(d);
    };

    const auto get_moving_piece = [&](const Square s)
    {
        if (move.is_promotion())
            return make_piece(us, move.promotion_piece());
        return piece_at(s);
    };

    const Piece capturedPiece = get_captured_piece(dst);
    const Piece movingPiece   = get_moving_piece(src);

    assert(color_of(movingPiece) == us);
    assert(capturedPiece == Piece::NONE || color_of(capturedPiece) == them);

    const Square previousEnPassantSquare = enPassantSquare_;
    clear_en_passant();

    if (move.is_castle()) {
        const auto [rookSrc, rookDst] = [dst]() -> std::pair<Square, Square>
        {
            if (dst == Square::G1)
                return {Square::H1, Square::F1};
            if (dst == Square::C1)
                return {Square::A1, Square::D1};
            if (dst == Square::G8)
                return {Square::H8, Square::F8};
            if (dst == Square::C8)
                return {Square::A8, Square::D8};
            return {Square::INVALID, Square::INVALID};
        }();

        remove_piece(src);
        remove_piece(rookSrc);
        set_piece(dst, movingPiece);
        set_piece(rookDst, make_piece(us, PieceType::ROOK));

        const auto mask = static_cast<CastlingRights>(
            (us == Color::WHITE) ? (WHITE_KING | WHITE_QUEEN) : (BLACK_KING | BLACK_QUEEN));
        unset_castling(mask);

        sideToMove_ = them;
        return;
    }

    if (capturedPiece == Piece::NONE) {
        set_piece(dst, movingPiece);
        remove_piece(src);
    }
    else {
        fiftyMoveRule_ = 0;
        if (move.is_en_passant()) {
            assert(previousEnPassantSquare == dst);
            set_piece(dst, movingPiece);
            remove_piece(src);

            if (us == Color::WHITE) {
                remove_piece(dst + Direction::SOUTH);
            }
            else {
                remove_piece(dst + Direction::NORTH);
            }
        }
        else {
            swap_piece(dst, movingPiece);
            remove_piece(src);
        }
    }

    if (type_of(movingPiece) == PieceType::PAWN) {
        const Rank srcRank = rank_of(src);
        const Rank dstRank = rank_of(dst);
        const int  diff    = std::abs(underlying(srcRank) - underlying(dstRank));

        if (diff == 2) {
            const Direction enPassantOffset =
                us == Color::WHITE ? Direction::SOUTH : Direction::NORTH;

            set_en_passant(dst + enPassantOffset);
        }

        fiftyMoveRule_ = 0;
    }

    sideToMove_ = ~sideToMove_;

    if (src == Square::A1 || dst == Square::A1 || movingPiece == Piece::W_KING) {
        unset_castling(WHITE_QUEEN);
    }
    if (src == Square::H1 || dst == Square::H1 || movingPiece == Piece::W_KING) {
        unset_castling(WHITE_KING);
    }
    if (src == Square::A8 || dst == Square::A8 || movingPiece == Piece::B_KING) {
        unset_castling(BLACK_QUEEN);
    }
    if (src == Square::H8 || dst == Square::H8 || movingPiece == Piece::B_KING) {
        unset_castling(BLACK_KING);
    }
}

void Position::generate_check_info()
{
    const Color    us               = sideToMove_;
    const Color    them             = ~sideToMove_;
    const Bitboard excludingOurKing = pieces() & ~pieces(us, PieceType::KING);
    const Bitboard ourKing          = pieces(us, PieceType::KING);
    const Square   ourKingSquare    = king_square(us);

    for (const Square src : pieces(them, PieceType::PAWN)) {
        const Bitboard attack          = generate_pawn_attacks(them, src, Bitboard::create_full());
        const bool     isAttackingKing = attack.intersects(ourKing);

        attackedSquares_ |= attack;
        if (isAttackingKing) {
            kingAttackers_ += 1;
            blockers_.set_square(src);
        }
    }

    for (const Square src : pieces(them, PieceType::KNIGHT)) {
        const Bitboard attack          = generate_attacks<PieceType::KNIGHT>(src, Bitboard{});
        const bool     isAttackingKing = attack.intersects(ourKing);

        attackedSquares_ |= attack;
        if (isAttackingKing) {
            kingAttackers_ += 1;
            blockers_.set_square(src);
        }
    }

    for (const Square src : pieces(them, PieceType::KING)) {
        const Bitboard attack = generate_attacks<PieceType::KING>(src, Bitboard{});
        attackedSquares_ |= attack;
    }

    for (const Square src : pieces(them, PieceType::BISHOP)) {
        const Bitboard attack          = generate_attacks<PieceType::BISHOP>(src, excludingOurKing);
        const bool     isAttackingKing = attack.intersects(ourKing);

        attackedSquares_ |= attack;
        if (isAttackingKing) {
            kingAttackers_ += 1;
            blockers_ |= Bitboard::generate_line(src, ourKingSquare);
        }
        else if (is_diagonal_to(src, ourKingSquare)) {
            const Bitboard pinRay           = Bitboard::generate_between(src, ourKingSquare);
            const int      piecesBetween    = (pinRay & pieces()).pop_count();
            const int      ourPiecesBetween = (pinRay & pieces(us)).pop_count();

            if (piecesBetween == 1 && ourPiecesBetween == 1) {
                pinRays_ |= pinRay;
            }
        }
    }

    for (const Square src : pieces(them, PieceType::ROOK)) {
        const Bitboard attack          = generate_attacks<PieceType::ROOK>(src, excludingOurKing);
        const bool     isAttackingKing = attack.intersects(ourKing);

        attackedSquares_ |= attack;
        if (isAttackingKing) {
            kingAttackers_ += 1;
            blockers_ |= Bitboard::generate_line(src, ourKingSquare);
        }
        else if (is_orthogonal_to(src, ourKingSquare)) {
            const Bitboard pinRay           = Bitboard::generate_between(src, ourKingSquare);
            const int      piecesBetween    = (pinRay & pieces()).pop_count();
            const int      ourPiecesBetween = (pinRay & pieces(us)).pop_count();

            if (piecesBetween == 1 && ourPiecesBetween == 1) {
                pinRays_ |= pinRay;
            }
        }
    }

    for (const Square src : pieces(them, PieceType::QUEEN)) {
        const Bitboard attack          = generate_attacks<PieceType::QUEEN>(src, excludingOurKing);
        const bool     isAttackingKing = attack.intersects(ourKing);

        attackedSquares_ |= attack;
        if (isAttackingKing) {
            kingAttackers_ += 1;
            blockers_ |= Bitboard::generate_line(src, ourKingSquare);
        }
        else if (is_diagonal_to(src, ourKingSquare) || is_orthogonal_to(src, ourKingSquare)) {
            const Bitboard pinRay           = Bitboard::generate_between(src, ourKingSquare);
            const int      piecesBetween    = (pinRay & pieces()).pop_count();
            const int      ourPiecesBetween = (pinRay & pieces(us)).pop_count();

            if (piecesBetween == 1 && ourPiecesBetween == 1) {
                pinRays_ |= pinRay;
            }
        }
    }
}

GameHistory::GameHistory(const std::string& fen, const std::vector<std::string>& moves)
{
    gameHistory_.push_back({.position = Position::from_fen(fen), .prevMove = Move{}});

    std::for_each(moves.begin(), moves.end(), [this](const std::string& move) { add_move(move); });
}

void GameHistory::add_move(const std::string& move)
{
    const Position& lastPosition = gameHistory_.back().position;
    const Move      parsedMove   = lastPosition.parse_move(move);
    add_move(parsedMove);
}

void GameHistory::add_move(Move move)
{
    const Position& lastPosition = gameHistory_.back().position;
    const SearchState newSearchState = {
        .position = {lastPosition, move, *this},
        .prevMove = move,
    };
    gameHistory_.emplace_back(newSearchState);
}

void GameHistory::pop_move()
{
    gameHistory_.pop_back();
}

const Position& GameHistory::pos() const
{
    return gameHistory_.back().position;
}

const SearchState& GameHistory::state() const
{
    return gameHistory_.back();
}

size_t GameHistory::ply() const
{
    return gameHistory_.size();
}

void GameHistory::begin_search()
{
    gameHistory_.reserve(gameHistory_.size() + 512);
}

void init_hash()
{
    std::mt19937_64 rng(std::random_device{}());

    for (auto& p : z::PieceHash) {
        for (auto& sq : p) {
            sq = rng();
        }
    }

    for (auto& c : z::CastleHash) {
        c = rng();
    }

    for (auto& i : z::EnPassantHash) {
        i = rng();
    }

    z::SideToMoveHash = rng();
}

} // namespace shellac
