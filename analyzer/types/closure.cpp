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

#include "vapor/analyzer/types/closure.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<std::vector<function *>> closure_type::get_candidates(lexer::token_type bracket) const
    {
        return make_ready_future(std::vector<function *>{ _function.get() });
    }

    void closure_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;

        auto type = codegen::ir::variable_type{
            U"__closure_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.closure_index++)), get_scope()->codegen_ir(ctx), 0, {}
        };

        auto scopes = get_scope()->codegen_ir(ctx);
        scopes.emplace_back(type.name, codegen::ir::scope_type::type);

        auto fn = _function->codegen_ir(ctx);
        fn.scopes = scopes;
        fn.parent_type = actual_type;
        type.members = { codegen::ir::member{ fn } };

        ctx.add_generated_function(_function.get());

        *actual_type = std::move(type);
    }
}
}
