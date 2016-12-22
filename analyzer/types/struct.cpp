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

#include "vapor/analyzer/types/struct.h"
#include "vapor/analyzer/expressions/variable.h"
#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/variables/struct.h"
#include "vapor/parser/expr.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct_type::~struct_type() = default;

    struct_type::struct_type(const parser::struct_literal & parse, scope * lex_scope) : type{ lex_scope }, _parse{ parse }
    {
        auto pair = make_promise<function *>();
        _aggregate_ctor_future = std::move(pair.future);
        _aggregate_ctor_promise = std::move(pair.promise);

        fmap(parse.members, [&](auto && member) {
            fmap(member,
                make_overload_set(
                    [&](const parser::declaration & decl) {
                        auto scope = _member_scope.get();
                        auto decl_stmt = preanalyze_member_declaration(decl, scope);
                        assert(scope == _member_scope.get());
                        _data_members_declarations.push_back(std::move(decl_stmt));

                        return unit{};
                    },

                    [&](const parser::function & func) {
                        assert(0);
                        return unit{};
                    }));

            return unit{};
        });
    }

    future<function *> struct_type::get_constructor(std::vector<const variable *> args) const
    {
        return _aggregate_ctor_future->then([&, args = std::move(args)](auto ret) {
            if (args.size() != _data_members.size() || !std::equal(args.begin(), args.end(), _data_members.begin(), [](auto && arg, auto && member) {
                    return arg->get_type() == member->get_type();
                }))
            {
                ret = nullptr;
            }

            return ret;
        });
    }

    void struct_type::generate_constructor()
    {
        _data_members = fmap(_data_members_declarations, [&](auto && member) { return member->declared_symbol()->get_variable(); });

        _aggregate_ctor = make_function(
            "struct type constructor", this, _data_members, [&](auto &&) -> codegen::ir::function { assert(!"TODO: codegen for struct constructors"); });

        _aggregate_ctor->set_eval([this](auto &&, const std::vector<variable *> & args) {
            auto repl = replacements{};
            auto arg_copies = fmap(args, [&](auto && arg) { return arg->clone_with_replacement(repl); });
            return make_ready_future<expression *>(make_variable_expression(make_struct_variable(this, std::move(arg_copies))).release());
        });

        _aggregate_ctor_promise->set(_aggregate_ctor.get());
    }
}
}
