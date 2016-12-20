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

#include <numeric>

#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/statements/block.h"
#include "vapor/analyzer/statements/function_declaration.h"
#include "vapor/analyzer/variables/overload_set.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    function_declaration::function_declaration(const parser::function & parse, scope * parent_scope) : _parse{ parse }, _scope{ parent_scope->clone_local() }
    {
        fmap(parse.arguments, [&](auto && arglist) {
            _argument_list = preanalyze_argument_list(arglist, _scope.get());
            return unit{};
        });
        _scope->close();

        _return_type = fmap(_parse.return_type, [&](auto && ret_type) { return preanalyze_expression(ret_type, _scope.get()); });
        _body = preanalyze_block(*_parse.body, _scope.get(), true);
        std::shared_ptr<overload_set> keep_count;
        auto symbol = parent_scope->get_or_init(_parse.name.string, [&] {
            keep_count = std::make_shared<overload_set>(_scope.get());
            return make_symbol(_parse.name.string, keep_count.get());
        });

        _overload_set = dynamic_cast<overload_set *>(symbol->get_variable())->shared_from_this();
    }

    void function_declaration::print(std::ostream & os, std::size_t indent) const
    {
        auto in = std::string(indent, ' ');
        os << in << "function declaration of `" << utf8(_parse.name.string) << "` at " << _parse.range << '\n';
        os << in << "arguments:\n";
        os << in << "{\n";
        fmap(_argument_list, [&, in = std::string(indent + 4, ' ')](auto && argument) {
            os << in << "argument `" << utf8(argument.name) << "` of type `" << argument.variable->get_type()->explain() << "`\n";
            return unit{};
        });
        os << in << "}\n";
        os << in << "return type: " << _function->return_type()->explain() << '\n';
        os << in << "{\n";
        _body->print(os, indent + 4);
        os << in << "}\n";
    }

    statement_ir function_declaration::_codegen_ir(ir_generation_context &) const
    {
        return {};
    }
}
}
