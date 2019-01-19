/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2017-2018 Michał "Griwes" Dominiak
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

#include <reaver/mayfly.h>

#include "vapor/lexer.h"
#include "vapor/parser.h"

namespace reaver
{
namespace vapor
{
    namespace parser
    {
        inline namespace _v1
        {
            template<typename T,
                typename U,
                typename = decltype(
                    print(std::declval<T>(), std::declval<std::ostream &>(), std::declval<print_context>()))>
            void report_different(std::stringstream & ss, T expected, U parsed)
            {
                print(expected, ss, {});
                ss << "\n != \n";
                print(parsed, ss, {});
                logger::dlog() << ss.str();
            }

            template<typename... Ts>
            void report_different(Ts &&...)
            {
            }

            template<typename T, typename F>
            auto test(std::u32string program, T expected, F && parser)
            {
                return [program = std::move(program),
                           expected = std::move(expected),
                           parser = std::move(parser)]() {
                    context ctx;
                    ctx.begin = lexer::iterator{ program.begin(), program.end(), std::nullopt };

                    std::stringstream ss;
                    for (auto it = ctx.begin; it != ctx.end; ++it)
                    {
                        ss << *it << '\n';
                    }

                    auto parsed = parser(ctx);
                    try
                    {
                        MAYFLY_REQUIRE(expected == parsed);
                    }
                    catch (...)
                    {
                        report_different(ss, expected, parsed);
                        throw;
                    }
                };
            }
        }
    }
}
}
