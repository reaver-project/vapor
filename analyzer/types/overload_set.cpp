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

#include "vapor/analyzer/expressions/expression.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/overload_set.h"
#include "vapor/codegen/ir/type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void overload_set_type::add_function(function * fn)
    {
        std::unique_lock<std::mutex> lock{ _functions_lock };

        if (std::find_if(_functions.begin(), _functions.end(), [&](auto && f) { return f->parameters() == fn->parameters(); }) != _functions.end())
        {
            assert(0);
        }

        _functions.push_back(fn);
    }

    future<std::vector<function *>> overload_set_type::get_candidates(lexer::token_type bracket) const
    {
        std::unique_lock<std::mutex> lock{ _functions_lock };

        if (bracket == lexer::token_type::round_bracket_open)
        {
            assert(_functions.size());
            return make_ready_future([&] { return _functions; }());
        }

        assert(0);
        return make_ready_future(std::vector<function *>{});
    }

    void overload_set_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;
        auto members = fmap(_functions, [&](auto && fn) {
            ctx.add_generated_function(fn);
            return codegen::ir::member{ fn->codegen_ir(ctx) };
        });
        auto type = codegen::ir::variable_type{ _codegen_name(ctx), get_scope()->codegen_ir(ctx), 0, std::move(members) };

        auto scopes = get_scope()->codegen_ir(ctx);
        scopes.emplace_back(type.name, codegen::ir::scope_type::type);

        fmap(type.members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](codegen::ir::function & fn) {
                        fn.scopes = scopes;
                        fn.parent_type = actual_type;

                        return unit{};
                    },
                    [&](auto &&) {
                        assert(0);
                        return unit{};
                    }));
            return unit{};
        });

        *actual_type = std::move(type);
    }

    void overload_set_type::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << "overload set type";
        os << styles::def << " @ " << styles::address << this << styles::def << ":\n";

        std::size_t idx = 0;
        for (auto && function : _functions)
        {
            function->print(os, ctx.make_branch(++idx == _functions.size()));
        }
    }
}
}
