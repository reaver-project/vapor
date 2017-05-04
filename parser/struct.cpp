/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016 Michał "Griwes" Dominiak
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
    namespace
    {
        struct named_struct
        {
            optional<lexer::token> name;
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
                ret.name = expect(ctx, lexer::token_type::identifier);
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

        decl.identifier = std::move(struct_.name.get());
        decl.range = struct_.definition.range;
        decl.rhs = expression{ struct_.definition.range, std::move(struct_.definition) };

        return decl;
    }

    void print(const struct_literal & lit, std::ostream & os, std::size_t indent)
    {
        auto in = std::string(indent, ' ');

        os << in << "`struct-literal` at " << lit.range << '\n';
        os << in << "{\n";
        fmap(lit.members, [&, in = std::string(indent + 4, ' ')](auto && member) {
            os << in << "{\n";
            fmap(member, [&](auto && decl) {
                print(decl, os, indent + 8);
                return unit{};
            });
            os << in << "}\n";
            return unit{};
        });

        os << in << "}\n";
    }
}
}
