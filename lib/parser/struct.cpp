/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/struct.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const struct_literal & lhs, const struct_literal & rhs)
    {
        return lhs.range == rhs.range && lhs.members == rhs.members;
    }

    namespace
    {
        struct named_struct
        {
            std::optional<identifier> name;
            struct_literal definition;
        };

        enum class struct_type
        {
            named,
            unnamed
        };

        named_struct parse_struct(context & ctx, struct_type type)
        {
            auto start = expect(ctx, lexer::token_type::struct_).range.start();

            named_struct ret;

            if (type == struct_type::named)
            {
                ret.name = parse_literal<lexer::token_type::identifier>(ctx);
            }

            expect(ctx, lexer::token_type::curly_bracket_open);

            while (!peek(ctx, lexer::token_type::curly_bracket_close))
            {
                ret.definition.members.push_back(parse_declaration(ctx, declaration_mode::member_declaration));
                expect(ctx, lexer::token_type::semicolon);
            }

            auto end = expect(ctx, lexer::token_type::curly_bracket_close).range.end();

            ret.definition.range = { start, end };

            return ret;
        }
    }

    struct_literal parse_struct_literal(context & ctx)
    {
        return parse_struct(ctx, struct_type::unnamed).definition;
    }

    declaration parse_struct_declaration(context & ctx)
    {
        auto struct_ = parse_struct(ctx, struct_type::named);

        declaration decl;

        decl.identifier = std::move(struct_.name.value());
        decl.range = struct_.definition.range;
        decl.rhs = expression{ struct_.definition.range, std::move(struct_.definition) };

        return decl;
    }

    void print(const struct_literal & lit, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "struct-literal";
        print_address_range(os, lit);
        os << '\n';

        std::size_t idx = 0;
        for (auto && member : lit.members)
        {
            fmap(member, [&](auto && decl) {
                print(decl, os, ctx.make_branch(++idx == lit.members.size()));
                return unit{};
            });
        }
    }
}
}
