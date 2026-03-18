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

#include "bitboard.h"

#include <array>
#include <optional>
#include <random>
#include <vector>

namespace shellac {
namespace {

constexpr std::array<uint64_t, 64> ROOK_MAGIC = {
    9403516606337777698ULL, 18032026129014784ULL,   9295464819560288322ULL, 36037595259760640ULL,
    9259409634463056512ULL, 180148384231981184ULL,  144115746421880841ULL,  648518692105749248ULL,
    9223512774951305880ULL, 4803370752596280417ULL, 563020853944384ULL,     5767422344214216768ULL,
    144396689099718912ULL,  2306405976767594512ULL, 72621098109370376ULL,   9570231152935044ULL,
    4719777082410795080ULL, 1153625742878384132ULL, 288373312967426564ULL,  599234105639432ULL,
    11259549235282944ULL,   282574555447848ULL,     4398609598474ULL,       18729081105481860ULL,
    9223513463685922816ULL, 594545521706668416ULL,  743252270346412032ULL,  3605730739842854913ULL,
    2252147706363912ULL,    2347505738190962720ULL, 1153484463150467075ULL, 18015231733437708ULL,
    9224568307661668387ULL, 3751501925577655360ULL, 563020853944384ULL,     4616471230956965889ULL,
    706590848059392ULL,     580817151657056ULL,     1297335764475973160ULL, 10394312343554097289ULL,
    708249770819592ULL,     144467036117073954ULL,  288265560792268928ULL,  9455800267341952ULL,
    2252147706363912ULL,    10995183419520ULL,      72621098109370376ULL,   9799833908337049612ULL,
    4791831104110887168ULL, 90107212354029376ULL,   708154310525056ULL,     297527848624718208ULL,
    2251817144561920ULL,    10995183419520ULL,      4512421508025344ULL,    140741783606400ULL,
    1771322305476039170ULL, 1771322305476039170ULL, 18182207193345ULL,      291045263626551306ULL,
    59109882819253250ULL,   579275570924486926ULL,  72059797448953860ULL,   282870908354610ULL,
};

constexpr std::array<uint64_t, 64> BISHOP_MAGIC = {
    5188727884117966976ULL, 2819220898381828ULL,    4796334378575725576ULL, 38316059305967680ULL,
    299342040662016ULL,     81223142831300640ULL,   144759536786871808ULL,  9295448056438720576ULL,
    144134987908908040ULL,  5190408912842539520ULL, 144838668909150208ULL,  2306010170565595136ULL,
    780567519852232992ULL,  145348845023281152ULL,  288230960956198912ULL,  9367522967800078337ULL,
    9232470220765989376ULL, 1441715245212107296ULL, 4613942363398865024ULL, 74309396032733504ULL,
    2320479727340486912ULL, 303148552071152144ULL,  70405554442240ULL,      9223934987378689024ULL,
    4621889486601721872ULL, 263480373773599233ULL,  2819422859559938ULL,    2900322566696636688ULL,
    432908651653660736ULL,  3658684323713847297ULL, 4616827343393261064ULL, 2306969187222422069ULL,
    7195480044864512ULL,    38282866723455232ULL,   594501573657493568ULL,  18084836107485704ULL,
    1242996799986208960ULL, 2891593543852296192ULL, 9570716148892740ULL,    9552416585277277698ULL,
    39129491775194120ULL,   310819911532284000ULL,  5066704803596288ULL,    4513633493846016ULL,
    3458843717379572545ULL, 9242583288135287296ULL, 884398916672422437ULL,  597294350526840873ULL,
    144759536786871808ULL,  1216046668464005121ULL, 90072277159264288ULL,   72075324208709768ULL,
    36028833006157832ULL,   2306558800007660032ULL, 9225677781558378496ULL, 2819220898381828ULL,
    9295448056438720576ULL, 9367522967800078337ULL, 288232989706467331ULL,  5621690803981158433ULL,
    1441152156715877888ULL, 9223829571672081536ULL, 144134987908908040ULL,  5188727884117966976ULL,
};

struct MagicEntry
{
    Bitboard      mask;
    std::uint64_t magic;
    std::uint8_t  indexBits;
};

std::array<MagicEntry, 64> rookMagics;
std::array<MagicEntry, 64> bishopMagics;

std::array<std::vector<Bitboard>, 64> rookMoves;
std::array<std::vector<Bitboard>, 64> bishopMoves;

enum Slider
{
    ROOK,
    BISHOP,
};

inline size_t magic_index(const MagicEntry& magicEntry, Bitboard blockers)
{
    blockers           = blockers & magicEntry.mask;
    std::uint64_t hash = std::uint64_t(blockers) * magicEntry.magic;
    return hash >> (64 - magicEntry.indexBits);
}

inline Bitboard get_rook_moves(Square square, Bitboard blockers)
{
    const MagicEntry& magic = rookMagics[underlying(square)];
    const auto&       moves = rookMoves[underlying(square)];
    return moves[magic_index(magic, blockers)];
}

inline Bitboard get_bishop_moves(Square square, Bitboard blockers)
{
    const MagicEntry& magic = bishopMagics[underlying(square)];
    const auto&       moves = bishopMoves[underlying(square)];
    return moves[magic_index(magic, blockers)];
}

template <Slider SLIDER>
Bitboard generate_slider_moves(Square src, Bitboard blockers);

template <>
Bitboard generate_slider_moves<Slider::BISHOP>(Square src, Bitboard blockers)
{
    constexpr std::array<std::pair<Direction, Direction>, 4> OFFSETS = {
        std::pair{Direction::NORTH, Direction::EAST},
        {Direction::NORTH, Direction::WEST},
        {Direction::SOUTH, Direction::EAST},
        {Direction::SOUTH, Direction::WEST},
    };

    Bitboard out{};

    for (const auto [a, b] : OFFSETS) {
        auto scanner = Bitboard{src};
        while (!scanner.is_empty()) {
            scanner = scanner.shift(a).shift(b);
            out |= scanner;
            if (!(scanner & blockers).is_empty()) {
                break;
            }
        }
    }

    return out;
}

template <>
Bitboard generate_slider_moves<Slider::ROOK>(Square src, Bitboard blockers)
{
    constexpr std::array<Direction, 4> OFFSETS = {
        Direction::NORTH,
        Direction::EAST,
        Direction::SOUTH,
        Direction::WEST,
    };

    Bitboard out{};

    for (const Direction direction : OFFSETS) {
        auto scanner = Bitboard{src};
        while (!scanner.is_empty()) {
            scanner = scanner.shift(direction);
            out |= scanner;
            if (!(scanner & blockers).is_empty()) {
                break;
            }
        }
    }

    return out;
}

template <Slider SLIDER>
Bitboard relevant_blockers(Square src)
{
    Bitboard moves = generate_slider_moves<SLIDER>(src, Bitboard{});

    Bitboard mask = moves;
    Bitboard srcBB(src);

    if (!srcBB.intersects(RANK_1)) {
        mask &= ~RANK_1;
    }
    if (!srcBB.intersects(RANK_8)) {
        mask &= ~RANK_8;
    }
    if (!srcBB.intersects(FILE_A)) {
        mask &= ~FILE_A;
    }
    if (!srcBB.intersects(FILE_H)) {
        mask &= ~FILE_H;
    }

    return mask;
}

std::vector<Bitboard> subsets(Bitboard mask)
{
    std::vector<Bitboard> out{};

    Bitboard subset = Bitboard{};

    do {
        out.emplace_back(subset);
        subset = Bitboard((std::uint64_t(subset) - std::uint64_t(mask)) & std::uint64_t(mask));
    }
    while (!subset.is_empty());

    return out;
}

template <Slider SLIDER>
std::optional<std::vector<Bitboard>> try_make_table(Square square, MagicEntry magicEntry)
{
    std::vector<Bitboard> table;
    for (int i = 0; i < 1 << magicEntry.indexBits; ++i) {
        table.emplace_back();
    }

    for (Bitboard blockers : subsets(magicEntry.mask)) {
        Bitboard  moves      = generate_slider_moves<SLIDER>(square, blockers);
        Bitboard& tableEntry = table[magic_index(magicEntry, blockers)];
        if (tableEntry == Bitboard()) {
            tableEntry = moves;
        }
        else if (tableEntry != moves) {
            return std::nullopt;
        }
    }

    return table;
}

template <Slider SLIDER>
std::pair<MagicEntry, std::vector<Bitboard>> find_magic(Square square, std::uint8_t indexBits,
                                                        std::uint64_t precomputedMagic = 0)
{
    const Bitboard  mask = relevant_blockers<SLIDER>(square);
    std::mt19937_64 rng{};
    while (true) {
        std::uint64_t magic = rng() & rng() & rng();
        if (precomputedMagic)
            magic = precomputedMagic;

        MagicEntry magicEntry{mask, magic, indexBits};

        std::optional<std::vector<Bitboard>> t;
        if ((t = try_make_table<SLIDER>(square, magicEntry))) {
            return {magicEntry, t.value()};
        }
    }
}
} // namespace

void init_magics()
{
    // Standard index bits for Rooks and Bishops

    // clang-format off
    static constexpr std::uint8_t ROOK_BITS[64] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12,
    };

    static constexpr std::uint8_t BISHOP_BITS[64] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6,
    };
    // clang-format on

    for (int i = 0; i < 64; ++i) {
        Square sq = static_cast<Square>(i);

        auto [rEntry, rTable] = find_magic<Slider::ROOK>(sq, ROOK_BITS[i], ROOK_MAGIC[i]);
        rookMagics[i]         = rEntry;
        rookMoves[i]          = std::move(rTable);

        auto [bEntry, bTable] = find_magic<Slider::BISHOP>(sq, BISHOP_BITS[i], BISHOP_MAGIC[i]);
        bishopMagics[i]       = bEntry;
        bishopMoves[i]        = std::move(bTable);
    }
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::KNIGHT>, Bitboard>
generate_attacks<PieceType::KNIGHT>(const Square src, Bitboard)
{
    const auto origin = Bitboard{src};
    Bitboard   out    = Bitboard{};
    out |= origin.shift(Direction::NORTH).shift(Direction::NORTH).shift(Direction::EAST);
    out |= origin.shift(Direction::NORTH).shift(Direction::NORTH).shift(Direction::WEST);
    out |= origin.shift(Direction::EAST).shift(Direction::EAST).shift(Direction::NORTH);
    out |= origin.shift(Direction::EAST).shift(Direction::EAST).shift(Direction::SOUTH);
    out |= origin.shift(Direction::SOUTH).shift(Direction::SOUTH).shift(Direction::EAST);
    out |= origin.shift(Direction::SOUTH).shift(Direction::SOUTH).shift(Direction::WEST);
    out |= origin.shift(Direction::WEST).shift(Direction::WEST).shift(Direction::NORTH);
    out |= origin.shift(Direction::WEST).shift(Direction::WEST).shift(Direction::SOUTH);
    return out;
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::BISHOP>, Bitboard>
generate_attacks<PieceType::BISHOP>(const Square src, const Bitboard blockers)
{
    return get_bishop_moves(src, blockers);
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::ROOK>, Bitboard>
generate_attacks<PieceType::ROOK>(const Square src, const Bitboard blockers)
{
    return get_rook_moves(src, blockers);
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::QUEEN>, Bitboard>
generate_attacks<PieceType::QUEEN>(const Square src, const Bitboard blockers)
{
    const Bitboard orthogonalAttacks = generate_attacks<PieceType::ROOK>(src, blockers);
    const Bitboard diagonalAttacks   = generate_attacks<PieceType::BISHOP>(src, blockers);
    return orthogonalAttacks | diagonalAttacks;
}

template <>
std::enable_if_t<!is_pawn_v<PieceType::KING>, Bitboard>
generate_attacks<PieceType::KING>(const Square src, const Bitboard)
{
    const Bitboard origin = Bitboard{src};
    Bitboard       out{};

    out |= origin.shift(Direction::NORTH);
    out |= origin.shift(Direction::SOUTH);
    out |= origin.shift(Direction::EAST);
    out |= origin.shift(Direction::WEST);
    out |= origin.shift(Direction::NORTH).shift(Direction::EAST);
    out |= origin.shift(Direction::NORTH).shift(Direction::WEST);
    out |= origin.shift(Direction::SOUTH).shift(Direction::EAST);
    out |= origin.shift(Direction::SOUTH).shift(Direction::WEST);

    return out;
}

Bitboard generate_pawn_destinations(const Color color, const Square src, const Bitboard blockers)
{
    const Rank      homeRank  = color == Color::WHITE ? Rank::R_2 : Rank::R_7;
    const Direction direction = color == Color::WHITE ? Direction::NORTH : Direction::SOUTH;
    Bitboard        out       = generate_pawn_attacks(color, src, blockers);

    const Bitboard oneAhead = Bitboard{src}.shift(direction);
    if ((blockers & oneAhead).is_empty()) {
        out |= oneAhead;

        if (rank_of(src) == homeRank) {
            const Bitboard twoAhead = oneAhead.shift(direction);
            if ((blockers & twoAhead).is_empty()) {
                out |= twoAhead;
            }
        }
    }

    return out;
}

Bitboard generate_pawn_attacks(const Color color, const Square src, const Bitboard blockers)
{
    const Direction direction = color == Color::WHITE ? Direction::NORTH : Direction::SOUTH;
    const Bitboard  lhs       = Bitboard{src}.shift(direction).shift(Direction::WEST);
    const Bitboard  rhs       = Bitboard{src}.shift(direction).shift(Direction::EAST);

    return (lhs | rhs) & blockers;
}
} // namespace shellac
