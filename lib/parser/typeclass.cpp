/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "vapor/parser/typeclass.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    bool operator==(const typeclass_literal & lhs, const typeclass_literal & rhs)
    {
        return lhs.range == rhs.range && lhs.members == rhs.members;
    }

    bool operator==(const typeclass_definition & lhs, const typeclass_definition & rhs)
    {
        return lhs.name == rhs.name && lhs.definition == rhs.definition;
    }

    namespace
    {
        struct named_typeclass
        {
            optional<identifier> name;
            typeclass_literal definition;
        };

        enum class typeclass_type
        {
            named,
            unnamed
        };

        named_typeclass parse_typeclass(context & ctx, typeclass_type type)
        {
            auto start = expect(ctx, lexer::token_type::typeclass).range.start();

            named_typeclass ret;

            if (type == typeclass_type::named)
            {
                ret.name = parse_literal<lexer::token_type::identifier>(ctx);
            }

            expect(ctx, lexer::token_type::curly_bracket_open);

            while (!peek(ctx, lexer::token_type::curly_bracket_close))
            {
                ret.definition.members.push_back(parse_function(ctx));
            }

            auto end = expect(ctx, lexer::token_type::curly_bracket_close).range.end();

            ret.definition.range = { start, end };

            return ret;
        }
    }

    typeclass_literal parse_typeclass_literal(context & ctx)
    {
        return parse_typeclass(ctx, typeclass_type::unnamed).definition;
    }

    typeclass_definition parse_typeclass_definition(context & ctx)
    {
        auto typeclass = parse_typeclass(ctx, typeclass_type::named);

        typeclass_definition decl;

        decl.name = std::move(typeclass.name.get());
        decl.definition = std::move(typeclass.definition);

        return decl;
    }

    void print(const typeclass_literal & lit, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "typeclass-literal";
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

    void print(const typeclass_definition & def, std::ostream & os, print_context ctx)
    {
        os << styles::def << ctx << styles::rule_name << "!!! typeclass-definition !!!:\n";

        auto name_ctx = ctx.make_branch(false);
        os << styles::def << name_ctx << styles::subrule_name << "name:\n";
        print(def.name, os, name_ctx.make_branch(true));

        auto def_ctx = ctx.make_branch(true);
        os << styles::def << def_ctx << styles::subrule_name << "definition:\n";
        print(def.definition, os, def_ctx.make_branch(true));
    }
}
}
