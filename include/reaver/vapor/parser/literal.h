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

#include <boost/variant.hpp>

#include "vapor/range.h"
#include "vapor/parser/helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            using literal = lexer::token;

            template<typename Context>
            auto parse_literal(Context & ctx)
            {
                literal ret;

                if (peek(ctx, lexer::token_type::string))
                {
                    ret = std::move(expect(ctx, lexer::token_type::string));
                }

                return ret;
            }

            void print(const literal & tok, std::ostream & os, std::size_t indent = 0)
            {
                auto in = std::string(indent, ' ');

                os << in << tok << '\n';
            }
        }}
    }
}
