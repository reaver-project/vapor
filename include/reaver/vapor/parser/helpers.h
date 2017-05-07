/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2017 Michał "Griwes" Dominiak
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

#include <reaver/exception.h>
#include <reaver/unit.h>

#include "../lexer/iterator.h"
#include "../lexer/token.h"
#include "../range.h"
#include "../utf8.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    class expectation_failure : public exception
    {
    public:
        expectation_failure(lexer::token_type expected, const std::u32string & actual, range_type & r) : exception{ logger::fatal }
        {
            *this << r << ": expected `" << lexer::token_types[+expected] << "`, got `" << utf8(actual) << "`";
        }

        expectation_failure(const std::string & str, const std::u32string & actual, range_type & r) : exception{ logger::fatal }
        {
            *this << r << ": expected " << str << ", got `" << utf8(actual) << "`";
        }

        expectation_failure(lexer::token_type expected) : exception{ logger::fatal }
        {
            *this << "expected `" << lexer::token_types[+expected] << "`, got end of file";
        }

        expectation_failure(const std::string & str) : exception{ logger::fatal }
        {
            *this << "expected " << str << ", got end of file";
        }
    };

    enum class operator_type
    {
        unary,
        binary
    };

    struct operator_context
    {
        lexer::token_type op;
        operator_type type;
    };

    enum class expression_special_modes
    {
        none,
        assignment,
        brace
    };

    struct context
    {
        lexer::iterator begin, end;
        std::vector<operator_context> operator_stack;
    };

    inline lexer::token expect(context & ctx, lexer::token_type expected)
    {
        if (ctx.begin->type != expected)
        {
            throw expectation_failure{ expected, ctx.begin->string, ctx.begin->range };
        }

        if (ctx.begin == ctx.end)
        {
            throw expectation_failure{ expected };
        }

        return std::move(*ctx.begin++);
    }

    inline optional<lexer::token &> peek(context & ctx)
    {
        if (ctx.begin != ctx.end)
        {
            return { *ctx.begin };
        }

        return {};
    }

    inline optional<lexer::token &> peek(context & ctx, lexer::token_type expected)
    {
        if (ctx.begin != ctx.end && ctx.begin->type == expected)
        {
            return { *ctx.begin };
        }

        return {};
    };
}
}
