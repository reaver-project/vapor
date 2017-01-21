/**
 * Vapor Compiler Licence
 *
 * Copyright © 2015-2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/function.h"
#include "vapor/parser/block.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    function parse_function(context & ctx)
    {
        function ret;

        auto start = expect(ctx, lexer::token_type::function).range.start();

        ret.name = expect(ctx, lexer::token_type::identifier);
        expect(ctx, lexer::token_type::round_bracket_open);
        if (peek(ctx) && peek(ctx)->type != lexer::token_type::round_bracket_close)
        {
            ret.arguments = parse_argument_list(ctx);
        }
        expect(ctx, lexer::token_type::round_bracket_close);

        if (peek(ctx, lexer::token_type::indirection))
        {
            expect(ctx, lexer::token_type::indirection);
            ret.return_type = parse_expression(ctx, expression_special_modes::brace);
        }

        if (peek(ctx, lexer::token_type::block_value))
        {
            ret.body = parse_single_statement_block(ctx);
        }
        else
        {
            ret.body = parse_block(ctx);
        }

        ret.range = { start, ret.body->range.end() };

        return ret;
    }

    void print(const function & f, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');
        auto in4 = std::string(indent + 4, ' ');

        os << in << "`function` at " << f.range << '\n';
        os << in << "{\n";
        os << in4 << f.name << '\n';
        fmap(f.arguments, [&](auto && arguments) {
            print(arguments, os, indent + 4);
            return unit{};
        });
        fmap(f.return_type, [&](auto && ret_type) {
            os << in4 << "return type:\n";
            os << in4 << "{\n";
            print(ret_type, os, indent + 8);
            os << in4 << "}\n";
            return unit{};
        });
        print(*f.body, os, indent + 4);
        os << in << "}\n";
    }
}
}
