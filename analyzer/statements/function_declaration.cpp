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

#include <numeric>

#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function_declaration.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    function_declaration::function_declaration(const parser::function & parse, scope * parent_scope) : _parse{ parse }, _scope{ parent_scope->clone_local() }
    {
        fmap(parse.parameters, [&](auto && param_list) {
            _parameter_list = preanalyze_parameter_list(param_list, _scope.get());
            return unit{};
        });
        _scope->close();

        _return_type = fmap(_parse.return_type, [&](auto && ret_type) { return preanalyze_expression(ret_type, _scope.get()); });
        _body = preanalyze_block(*_parse.body, _scope.get(), true);
        std::shared_ptr<overload_set> keep_count;
        auto symbol = parent_scope->get_or_init(_parse.name.value.string, [&] {
            keep_count = std::make_shared<overload_set>(_scope.get());
            return make_symbol(_parse.name.value.string, keep_count.get());
        });

        _overload_set = symbol->get_expression()->as<overload_set>()->shared_from_this();
    }

    void function_declaration::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "function-declaration";
        print_address_range(os, this);
        os << ' ' << styles::string_value << utf8(_parse.name.value.string) << '\n';

        if (_parameter_list.size())
        {
            auto params_ctx = ctx.make_branch(false);
            os << styles::def << params_ctx << styles::subrule_name << "parameters:\n";

            std::size_t idx = 0;
            for (auto && param : _parameter_list)
            {
                param->print(os, params_ctx.make_branch(++idx == _parameter_list.size()));
            }
        }

        auto return_type_ctx = ctx.make_branch(false);
        os << styles::def << return_type_ctx << styles::subrule_name << "return type:\n";
        _function->return_type_expression()->print(os, return_type_ctx.make_branch(true));

        auto body_ctx = ctx.make_branch(true);
        os << styles::def << body_ctx << styles::subrule_name << "body:\n";
        _body->print(os, body_ctx.make_branch(true));
    }

    statement_ir function_declaration::_codegen_ir(ir_generation_context &) const
    {
        return {};
    }
}
}
