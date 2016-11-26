/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2016 Michał "Griwes" Dominiak
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

#include <memory>

#include <reaver/prelude/monad.h>

#include "../helpers.h"
#include "../statements/statement.h"
#include "../variables/variable.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct expression;
    struct expression_list;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class scope;

    class expression : public statement
    {
    public:
        expression() = default;
        virtual ~expression() = default;

        expression(std::unique_ptr<variable> var) : _variable{ std::move(var) }
        {
        }

        // do NOT abuse the fact this is virtual!
        virtual variable * get_variable() const
        {
            if (!_variable)
            {
                assert(!"someone tried to get variable before analyzing... or forgot to set variable from analyze");
            }

            return _variable.get();
        }

        type * get_type()
        {
            auto var = get_variable();

            if (!var)
            {
                assert(!"someone tried to get type before analyzing... or forgot to set variable from analyze");
            }

            return var->get_type();
        }

        std::unique_ptr<expression> clone_expr_with_replacement(replacements & repl) const
        {
            auto ret = _clone_expr_with_replacement(repl);
            repl.expressions[this] = ret.get();
            return ret;
        }

        future<expression *> simplify_expr(simplification_context & ctx)
        {
            return ctx.get_future_or_init(this, [&]() { return make_ready_future().then([this, &ctx]() { return _simplify_expr(ctx); }); });
        }

    protected:
        virtual std::unique_ptr<statement> _clone_with_replacement(replacements & repl) const override
        {
            return _clone_expr_with_replacement(repl);
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const = 0;

        virtual future<statement *> _simplify(simplification_context & ctx) override final
        {
            return simplify_expr(ctx).then([&](auto && simplified) -> statement * { return simplified; });
        }

        virtual future<expression *> _simplify_expr(simplification_context &) = 0;

        void _set_variable(std::unique_ptr<variable> var)
        {
            assert(var);
            assert(!_variable);
            _variable = std::move(var);
        }

        void _set_variable(variable * ptr, simplification_context & ctx)
        {
            replace_uptr(_variable, ptr, ctx);
        }

    private:
        std::unique_ptr<variable> _variable;
    };

    std::unique_ptr<expression> preanalyze_expression(const parser::expression & expr, scope * lex_scope);
}
}
