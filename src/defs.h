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

#ifndef SHELLAC_DEFS_H
#define SHELLAC_DEFS_H
#include <climits>
#include <cstdint>
#include <string_view>

namespace shellac {

#if defined(__clang__)
#define CLANG
#define CXX_COMPILER_NAME "clang++"
#elif defined(__GNUC__) || defined(__GNUG__)
#define GCC
#define CXX_COMPILER_NAME "gcc"
#elif defined(_MSC_VER)
#define MSVC
#define CXX_COMPILER_NAME "MSVC"
#else
#define CXX_COMPILER_NAME "unknown"
#endif

#define SHELLAC_RELEASE

#ifdef SHELLAC_RELEASE
constexpr const char* BuildIdentifier = "2";
#elif defined(BUILD_IDENTIFIER)
constexpr const char* BuildIdentifier = BUILD_IDENTIFIER "-" CXX_COMPILER_NAME;
#else
constexpr const char* BuildIdentifier = "unknown-build";
#endif

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr int ctz(T t)
{
    using U = std::make_unsigned_t<T>;

    if (t == 0) {
        return CHAR_BIT * sizeof(T);
    }

    U   scanner = 1;
    U   value   = static_cast<U>(t);
    int count   = 0;

    while (scanner) {
        if (scanner & value) {
            return count;
        }
        count++;
        scanner <<= 1;
    }
    return count;
}

#ifdef GCC
template <>
constexpr int ctz<std::uint32_t>(const std::uint32_t t)
{
    return __builtin_ctz(t);
}

template <>
constexpr int ctz<std::uint64_t>(const std::uint64_t t)
{
    return __builtin_ctzll(t);
}
#endif

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
constexpr int pop_count(T t)
{
    using U = std::make_unsigned_t<T>;

    U   scanner = 1;
    U   value   = static_cast<U>(t);
    int count   = 0;

    while (scanner) {
        if (scanner & value) {
            ++count;
        }

        scanner <<= 1;
    }

    return count;
}

#if defined(GCC)
template <>
constexpr int pop_count<std::uint64_t>(std::uint64_t t)
{
    return __builtin_popcountll(t);
}

template <>
constexpr int pop_count<std::uint32_t>(std::uint32_t t)
{
    return __builtin_popcount(t);
}
#endif

template <typename T, typename = std::enable_if_t<std::is_enum_v<T>>>
constexpr std::underlying_type_t<T> underlying(T enumValue)
{
    return static_cast<std::underlying_type_t<T>>(enumValue);
}

template <typename T>
constexpr int signum(T x, const std::false_type is_signed) {
    (void)is_signed;
    return T(0) < x;
}

template <typename T>
constexpr int signum(T x, const std::true_type is_signed) {
    (void)is_signed;
    return (T(0) < x) - (x < T(0));
}

template <typename T>
constexpr int signum(T x) {
    return signum(x, std::is_signed<T>());
}

template <typename T>
constexpr T from_char(char c) = delete;

template <typename T>
constexpr T from_string(std::string_view s) = delete;

} // namespace shellac

#endif // SHELLAC_DEFS_H
