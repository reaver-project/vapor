/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016, 2018 Michał "Griwes" Dominiak
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

#include "expression.h"

namespace reaver::vapor::proto
{
struct import_;
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class entity;

    class import_expression : public expression
    {
    public:
        import_expression(ast_node node, entity * module) : _node{ std::move(node) }, _module{ module }
        {
        }

        virtual void print(std::ostream &, print_context) const override;

    private:
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override
        {
            assert(0);
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override
        {
            assert(0);
        }

        ast_node _node;
        entity * _module;
    };

    enum class import_mode
    {
        statement,
        expression
    };
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct import_expression;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;

    std::unique_ptr<import_expression> preanalyze_import(precontext & ctx,
        const parser::import_expression & parse,
        scope * lex_scope,
        import_mode mode = import_mode::expression);
}
}
