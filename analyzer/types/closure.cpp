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

#include "vapor/analyzer/types/closure.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/codegen/ir/type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<function *> closure_type::get_overload(lexer::token_type bracket, std::vector<const type *> args) const
    {
        if (args.size() == _function->arguments().size()
            && std::inner_product(args.begin(), args.end(), _function->arguments().begin(), true, std::logical_and<>(), [](auto && type, auto && var) {
                   return type == var->get_type();
               }))
        {
            return make_ready_future(_function.get());
        }

        return make_ready_future(static_cast<function *>(nullptr));
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
