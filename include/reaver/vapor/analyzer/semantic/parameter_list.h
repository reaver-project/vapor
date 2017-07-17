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

#include "../../parser/parameter_list.h"
#include "../expressions/expression.h"
#include "../expressions/expression_ref.h"
#include "../expressions/type.h"
#include "../symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class parameter : public expression
    {
    public:
        parameter(const parser::parameter & parse, scope * lex_scope)
            : _parse{ parse }, _name{ parse.name.value.string }, _type_expression{ preanalyze_expression(parse.type, lex_scope) }
        {
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::rule_name << "parameter";
            print_address_range(os, this);
            os << styles::string_value << utf8(_name) << '\n';

            auto type_expr_ctx = ctx.make_branch(true);
            os << styles::def << type_expr_ctx << styles::subrule_name << "type expression:\n";
            _type_expression->print(os, type_expr_ctx.make_branch(true));
        }

        auto parse() const
        {
            return _parse;
        }

    private:
        virtual future<> _analyze(analysis_context & ctx) override
        {
            return _type_expression->analyze(ctx).then([&] {
                auto type_value = _type_expression->as<type_expression>();
                assert(type_value);
                _set_type(type_value->get_value());
            });
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const override
        {
            return make_expression_ref(const_cast<parameter *>(this));
        }

        virtual statement_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            auto var = codegen::ir::make_variable(get_type()->codegen_type(ctx));
            var->parameter = true;
            return { codegen::ir::instruction{
                none, none, { boost::typeindex::type_id<codegen::ir::materialization_instruction>() }, {}, { std::move(var) } } };
        }

        const parser::parameter & _parse;

        std::u32string _name;
        std::unique_ptr<expression> _type_expression;
    };

    using parameter_list = std::vector<std::unique_ptr<parameter>>;

    inline parameter_list preanalyze_parameter_list(const parser::parameter_list & arglist, scope * lex_scope)
    {
        return fmap(arglist.parameters, [&](auto && arg) {
            auto param = std::make_unique<parameter>(arg, lex_scope);

            auto symb = make_symbol(arg.name.value.string, param.get());
            lex_scope->init(arg.name.value.string, std::move(symb));

            return param;
        });
    }
}
}
