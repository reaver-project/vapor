/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016, 2018-2019 Michał "Griwes" Dominiak
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 **/

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace reaver::vapor
{
inline namespace _v1
{
    struct position
    {
        position()
        {
        }
        position(const position &) = default;
        position(position &&) = default;
        position & operator=(const position &) = default;
        position & operator=(position &&) = default;

        position(std::size_t offset, std::optional<std::string_view> file_path = std::nullopt)
            : offset{ offset }, file_path{ file_path }
        {
        }

        std::size_t operator-(const position & rhs) const
        {
            // throw assert(offset > rhs.offset)
            return offset - rhs.offset;
        }

        position & operator+=(std::size_t len)
        {
            offset += len;
            column += len;
            return *this;
        }

        position operator+(std::size_t len) const
        {
            position ret(*this);
            ret += len;
            return ret;
        }

        std::size_t offset = 0;
        std::size_t line = 1;
        std::size_t column = 1;
        std::optional<std::string_view> file_path;
    };

    inline bool operator!=(const position & lhs, const position & rhs)
    {
        return lhs.offset != rhs.offset || lhs.file_path != rhs.file_path;
    }

    inline bool operator==(const position & lhs, const position & rhs)
    {
        return !(lhs != rhs);
    }
}
}
