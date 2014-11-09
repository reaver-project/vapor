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

#include <type_traits>
#include <vector>

#include <reaver/error.h>

#include "vapor/lexer/token.h"
#include "vapor/parser/module.h"
#include "vapor/parser/statement.h"
#include "vapor/parser/helpers.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            class ast
            {
            public:
                template<typename Iterator, typename std::enable_if<std::is_same<typename std::iterator_traits<Iterator>::value_type, lexer::token>::value, int>::type = 0>
                ast(Iterator begin, Iterator end = {})
                {
                    auto ctx = make_context(begin, end);

                    while (ctx.begin != ctx.end)
                    {
                        _modules.push_back(parse_module(ctx));
                    }
                }

                auto begin()
                {
                    return _modules.begin();
                }

                auto begin() const
                {
                    return _modules.begin();
                }

                auto end()
                {
                    return _modules.end();
                }

                auto end() const
                {
                    return _modules.end();
                }

            private:
                std::vector<module> _modules;
            };

            std::ostream & operator<<(std::ostream & os, const ast & ast)
            {
                for (auto && module : ast)
                {
                    os << "{\n";
                    print(module, os, 4);
                    os <<  "}\n";
                }

                return os;
            }
        }}
    }
}