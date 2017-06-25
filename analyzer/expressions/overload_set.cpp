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
#include "vapor/analyzer/statements/function_declaration.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> overload_set::_clone_expr_with_replacement(replacements & repl) const
    {
        assert(0);
    }

    statement_ir overload_set::_codegen_ir(ir_generation_context & ctx) const
    {
        auto var = codegen::ir::make_variable(_type->codegen_type(ctx));
        var->scopes = _type->get_scope()->codegen_ir(ctx);
        // return { std::move(var) };
        assert(!"do a pass value instruction here");
    }

    void overload_set::add_function(function_declaration * decl)
    {
        _function_decls.push_back(decl);
        _type->add_function(decl->get_function());
    }
}
}
