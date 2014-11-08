/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014 Michał "Griwes" Dominiak
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

#include <ostream>

#include "vapor/position.h"

namespace reaver
{
    namespace vapor { inline namespace _v1
    {
        class range
        {
        public:
            range() = default;
            range(const range &) = default;
            range(range &&) = default;
            range & operator=(const range &) = default;
            range & operator=(range &&) = default;

            range(position s, position e) : _start{ std::move(s) }, _end{ std::move(e) }
            {
            // throw: assert(s.offset <= e.offset);
            }

            const position & start() const
            {
                return _start;
            }

            const position & end() const
            {
                return _end;
            }

        private:
            position _start;
            position _end;
        };

        bool operator==(const range & lhs, const range & rhs)
        {
            return lhs.start() == rhs.start() && lhs.end() == rhs.end();
        }

        std::ostream & operator<<(std::ostream & os, const range & r)
        {
            if (r.end() - r.start() > 1)
            {
                return os << r.start().line << ":" << r.start().column << " (" << r.start().offset << ") - " << r.end().line << ":" << r.end().column << " (" <<  r.end().offset << ")";
            }

            return os << r.start().line << ":" << r.start().column << " (" << r.start().offset << ")";
        }
    }}
}