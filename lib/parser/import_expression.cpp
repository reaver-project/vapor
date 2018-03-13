/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2018 Michał "Griwes" Dominiak
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

#include <reaver/variant.h>

#include "vapor/parser/import_expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const import_expression & lhs, const import_expression & rhs)
    {
        return lhs.range == rhs.range && lhs.module_name == rhs.module_name;
    }

    import_expression parse_import_expression(context & ctx)
    {
        import_expression ret;

        auto start = expect(ctx, lexer::token_type::import).range.start();
        if (peek(ctx, lexer::token_type::string))
        {
            ret.module_name = parse_literal<lexer::token_type::string>(ctx);
        }
        else
        {
            ret.module_name = parse_id_expression(ctx);
        }

        fmap(ret.module_name, [&](const auto & elem) {
            ret.range = { start, elem.range.end() };
            return unit{};
        });

        return ret;
    }

    void print(const import_expression & expr, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "import-expression";
        print_address_range(os, expr);
        os << '\n';

        auto name_ctx = ctx.make_branch(true);
        os << styles::def << name_ctx << styles::subrule_name << "name:\n";
        fmap(expr.module_name, [&](const auto & elem) {
            print(elem, os, name_ctx.make_branch(true));
            return unit{};
        });
    }
}
}
