/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/parser/expr.h"

#include "expressions/type.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void type_expression::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::rule_name << "type-expression";
        print_address_range(os, this);
        os << '\n';

        auto type_ctx = ctx.make_branch(true);
        os << styles::def << type_ctx << styles::subrule_name << "value:\n";
        _type->print(os, type_ctx.make_branch(true));
    }

    bool type_expression::_is_equal(const expression * rhs) const
    {
        auto type_rhs = rhs->as<type_expression>();
        return type_rhs && _type == type_rhs->_type;
    }

    std::unique_ptr<expression> type_expression::_clone_expr(replacements & repl) const
    {
        auto type = repl.try_get_replacement(_type);
        if (!type)
        {
            type = _type;
        }

        return std::make_unique<type_expression>(type);
    }

    constant_init_ir type_expression::_constinit_ir(ir_generation_context & ctx) const
    {
        return _type->codegen_type(ctx);
    }

    std::unique_ptr<google::protobuf::Message> type_expression::_generate_interface() const
    {
        return _type->generate_interface();
    }
}
}
