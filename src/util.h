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

#ifndef SHELLAC_UTIL_H
#define SHELLAC_UTIL_H
#include "defs.h"

namespace shellac {

template <typename T, usize kElementCount>
class StackVector
{
public:
    StackVector() = default;

    void push_back(T value)
    {
        backingArray_[size_++] = value;
    }

    T* begin()
    {
        return backingArray_.data();
    }

    T* end()
    {
        return backingArray_.data() + size_;
    }

    const T* begin() const
    {
        return backingArray_.data();
    }

    const T* end() const
    {
        return backingArray_.data() + size_;
    }

    const T* cbegin() const
    {
        return backingArray_.data();
    }

    const T* cend() const
    {
        return backingArray_.data() + size_;
    }

private:
    std::array<T, kElementCount> backingArray_{};
    usize                        size_{};
};

} // namespace shellac

#endif // SHELLAC_UTIL_H
