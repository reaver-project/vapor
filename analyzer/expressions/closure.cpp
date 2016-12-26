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

#include "vapor/analyzer/expressions/closure.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/closure.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    closure::closure(const parser::lambda_expression & parse, scope * lex_scope) : _parse{ parse }, _scope{ lex_scope->clone_local() }
    {
        fmap(parse.arguments, [&](auto && arglist) {
            _argument_list = preanalyze_argument_list(arglist, _scope.get());
            ;
            return unit{};
        });
        _scope->close();

        _return_type = fmap(_parse.return_type, [&](auto && ret_type) { return preanalyze_expression(ret_type, _scope.get()); });
        _body = preanalyze_block(parse.body, _scope.get(), true);
    }

    void closure::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');
        os << in << "closure at " << _parse.range << '\n';
        assert(!_parse.captures);
        os << in << "{\n";
        fmap(_argument_list, [&, in = std::string(indent + 4, ' ')](auto && argument) {
            os << in << "argument `" << utf8(argument.name) << "` of type `" << argument.variable->get_type()->explain() << "`\n";
            return unit{};
        });
        os << in << "}\n";
        os << in << "return type: " << _body->return_type()->explain() << '\n';
        os << in << "{\n";
        _body->print(os, indent + 4);
        os << in << "}\n";
    }

    statement_ir closure::_codegen_ir(ir_generation_context & ctx) const
    {
        auto var = codegen::ir::make_variable(get_variable()->get_type()->codegen_type(ctx));
        var->scopes = _type->get_scope()->codegen_ir(ctx);
        return { codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::materialization_instruction>() }, {}, { std::move(var) } } };
    }
}
}
