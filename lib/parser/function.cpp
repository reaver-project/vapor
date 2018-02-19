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

#include "vapor/parser/function.h"
#include "vapor/parser/block.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const function_declaration & lhs, const function_declaration & rhs)
    {
        return lhs.range == rhs.range && lhs.name == rhs.name && lhs.parameters == rhs.parameters && lhs.return_type == rhs.return_type;
    }

    bool operator==(const function_definition & lhs, const function_definition & rhs)
    {
        return lhs.range == rhs.range && lhs.signature == rhs.signature && *lhs.body == rhs.body;
    }

    function_declaration parse_function_declaration(context & ctx, parameter_type_mode mode)
    {
        function_declaration ret;

        if (peek(ctx, lexer::token_type::export_))
        {
            ret.export_ = expect(ctx, lexer::token_type::export_);
        }

        auto start = expect(ctx, lexer::token_type::function).range.start();

        ret.name = parse_literal<lexer::token_type::identifier>(ctx);
        expect(ctx, lexer::token_type::round_bracket_open);
        if (peek(ctx) && peek(ctx)->type != lexer::token_type::round_bracket_close)
        {
            ret.parameters = parse_parameter_list(ctx, mode);
        }
        auto end = expect(ctx, lexer::token_type::round_bracket_close).range.end();

        if (peek(ctx, lexer::token_type::indirection))
        {
            expect(ctx, lexer::token_type::indirection);
            ret.return_type = parse_expression(ctx, expression_special_modes::brace);
            end = ret.return_type->range.end();
        }

        ret.range = { start, end };

        return ret;
    }

    function_definition parse_function_definition(context & ctx, parameter_type_mode mode)
    {
        return parse_function_definition(ctx, std::nullopt, mode);
    }

    function_definition parse_function_definition(context & ctx, std::optional<function_declaration> decl, parameter_type_mode mode)
    {
        function_definition ret;

        ret.signature = decl ? std::move(*decl) : parse_function_declaration(ctx, mode);

        if (peek(ctx, lexer::token_type::block_value))
        {
            ret.body = parse_single_statement_block(ctx);
        }
        else
        {
            ret.body = parse_block(ctx);
        }

        ret.range = { ret.signature.range.start(), ret.body->range.end() };

        return ret;
    }

    std::variant<function_declaration, function_definition> parse_function(context & ctx)
    {
        auto decl = parse_function_declaration(ctx);

        if (peek(ctx, lexer::token_type::semicolon))
        {
            expect(ctx, lexer::token_type::semicolon);
            return { std::move(decl) };
        }

        return { parse_function_definition(ctx, std::move(decl)) };
    }

    void print(const function_declaration & f, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "function-declaration";
        print_address_range(os, f);

        auto name_ctx = ctx.make_branch(false);
        os << '\n' << name_ctx << styles::subrule_name << "name:\n";
        print(f.name, os, name_ctx.make_branch(true));

        fmap(f.return_type, [&](auto && ret) {
            auto ret_ctx = ctx.make_branch(!f.parameters);
            os << ret_ctx << styles::subrule_name << "return-type:\n";
            print(ret, os, ret_ctx.make_branch(true));
            return unit{};
        });

        fmap(f.parameters, [&](auto && params) {
            print(params, os, ctx.make_branch(true));
            return unit{};
        });
    }

    void print(const function_definition & f, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "function-definition";
        print_address_range(os, f);
        os << '\n';

        print(f.signature, os, ctx.make_branch(false));
        print(*f.body, os, ctx.make_branch(true));
    }
}
}
