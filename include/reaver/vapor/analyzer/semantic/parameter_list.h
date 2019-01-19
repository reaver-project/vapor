/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include "../expressions/expression.h"
#include "../expressions/expression_ref.h"
#include "../expressions/type.h"
#include "../semantic/symbol.h"
#include "instance_context.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class parameter : public expression
    {
    public:
        parameter(ast_node parse, std::u32string name, std::unique_ptr<expression> type);

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::rule_name << "parameter";
            print_address_range(os, this);
            os << ' ' << styles::string_value << utf8(_name) << '\n';

            auto type_expr_ctx = ctx.make_branch(true);
            os << styles::def << type_expr_ctx << styles::subrule_name << "type expression:\n";
            _type_expression->print(os, type_expr_ctx.make_branch(true));
        }

        virtual void set_template_parameters(std::vector<parameter *> params) override
        {
            assert(!_template_params);
            _template_params = std::move(params);
        }

        expression * get_type_expression() const
        {
            return _type_expression.get();
        }

    private:
        virtual future<> _analyze(analysis_context & ctx) override
        {
            return _type_expression->analyze(ctx).then([&] {
                if (!_template_params)
                {
                    auto type_value = _type_expression->as<type_expression>();
                    assert(type_value);
                    this->_set_type(type_value->get_value());
                }
            });
        }

        virtual future<expression *> _simplify_expr(recursive_context ctx) override
        {
            return _type_expression->simplify_expr(ctx).then([&ctx = ctx.proper, this](auto && simpl)->expression * {
                replace_uptr(_type_expression, simpl, ctx);
                return this;
            });
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            if (_template_params)
            {
                return std::make_unique<parameter>(get_ast_info().value(), _name, repl.claim(_type_expression.get()));
            }

            return make_expression_ref(const_cast<parameter *>(this), get_ast_info());
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            auto var = codegen::ir::make_variable(get_type()->codegen_type(ctx));
            var->parameter = true;
            return { codegen::ir::instruction{
                std::nullopt, std::nullopt, { boost::typeindex::type_id<codegen::ir::materialization_instruction>() }, {}, { std::move(var) } } };
        }

        virtual std::unique_ptr<google::protobuf::Message> _generate_interface() const override
        {
            assert(0);
        }

        std::u32string _name;
        std::unique_ptr<expression> _type_expression;
        std::optional<std::vector<parameter *>> _template_params;
    };

    using parameter_list = std::vector<std::unique_ptr<parameter>>;
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct parameter_list;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;

    parameter_list preanalyze_parameter_list(precontext & ctx,
        const parser::parameter_list & param_list,
        scope * lex_scope,
        std::optional<instance_function_context> = std::nullopt);
}
}
