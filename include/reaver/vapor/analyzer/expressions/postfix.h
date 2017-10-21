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

#pragma once

#include "expression.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct postfix_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class scope;
    class function;

    class postfix_expression : public expression
    {
    public:
        postfix_expression(const parser::postfix_expression & parse, scope * lex_scope);

        virtual void print(std::ostream & os, print_context ctx) const override;

        future<expression *> get_base_expression(analysis_context & ctx) const
        {
            return _base_expr->analyze(ctx).then([&] { return _base_expr.get(); });
        }

    private:
        postfix_expression(const postfix_expression & other) : _modifier{ other._modifier }, _accessed_member{ other._accessed_member }
        {
        }

        static auto _get_replacement_helper()
        {
            return [](auto && self) {
                if (self->_accessed_member)
                {
                    return self->_referenced_expression.get()->_get_replacement();
                }

                if (self->_modifier)
                {
                    return self->_call_expression->_get_replacement();
                }

                return self->_base_expr->_get_replacement();
            };
        }

        virtual expression * _get_replacement() override
        {
            auto repl = _get_replacement_helper()(this);
            assert(repl);
            return repl;
        }

        virtual const expression * _get_replacement() const override
        {
            auto repl = _get_replacement_helper()(this);
            assert(repl);
            return repl;
        }

        virtual future<> _analyze(analysis_context &) override;
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override;
        virtual future<expression *> _simplify_expr(recursive_context) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        virtual bool _is_equal(const expression * rhs) const override
        {
            if (_referenced_expression)
            {
                return _referenced_expression.get()->is_equal(rhs);
            }

            if (_call_expression)
            {
                return _call_expression->is_equal(rhs);
            }

            return _base_expr->is_equal(rhs);
        }

        scope * _scope = nullptr;
        std::unique_ptr<expression> _base_expr;
        optional<lexer::token_type> _modifier;
        std::vector<std::unique_ptr<expression>> _arguments;
        std::unique_ptr<expression> _call_expression;

        optional<std::u32string> _accessed_member;
        optional<expression *> _referenced_expression;
    };

    inline std::unique_ptr<postfix_expression> preanalyze_postfix_expression(const parser::postfix_expression & parse, scope * lex_scope)
    {
        return std::make_unique<postfix_expression>(parse, lex_scope);
    }
}
}
