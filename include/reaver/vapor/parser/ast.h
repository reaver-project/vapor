/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2018 Michał "Griwes" Dominiak
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

#include <vector>

#include "../lexer/iterator.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct import_expression;
    struct module;

    struct ast
    {
        ast() = default;
        ast(const ast &) = delete;
        ast(ast &&) = default;
        ast & operator=(const ast &) = delete;
        ast & operator=(ast &&) = default;

        std::vector<import_expression> global_imports;
        std::vector<module> module_definitions;
    };

    ast parse_ast(lexer::iterator begin, lexer::iterator end = {});
    std::ostream & operator<<(std::ostream & os, const ast & ast);
}
}
