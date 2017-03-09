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
        auto ctor_pair = make_promise<function *>();
        _aggregate_ctor_future = std::move(ctor_pair.future);
        _aggregate_ctor_promise = std::move(ctor_pair.promise);

        auto copy_pair = make_promise<function *>();
        _aggregate_copy_ctor_future = std::move(copy_pair.future);
        _aggregate_copy_ctor_promise = std::move(copy_pair.promise);

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

        _member_scope->close();
    }

    void struct_type::generate_constructors()
    {
        _data_members =
            fmap(_data_members_declarations, [&](auto && member) { return static_cast<member_variable *>(member->declared_symbol()->get_variable()); });

        auto data_members = fmap(_data_members, [](auto && member) -> variable * { return member; });

        _aggregate_ctor = make_function("struct type constructor", get_expression(), data_members, [&](auto && ctx) -> codegen::ir::function {
            auto ir_type = this->codegen_type(ctx);
            auto args = fmap(_data_members, [&](auto && member) { return get<codegen::ir::value>(member->codegen_ir(ctx)); });
            auto result = codegen::ir::make_variable(ir_type);

            return { U"constructor",
                this->get_scope()->codegen_ir(ctx),
                fmap(args, [](auto && arg) { return get<std::shared_ptr<codegen::ir::variable>>(arg); }),
                result,
                { codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::aggregate_init_instruction>() }, args, result } } };
        });

        _aggregate_ctor->set_eval([this](auto &&, const std::vector<variable *> & args) {
            if (!std::equal(args.begin(), args.end(), _data_members.begin(), [](auto && arg, auto && member) { return arg->get_type() == member->get_type(); }))
            {
                assert(0);
            }

            auto repl = replacements{};
            auto arg_copies = fmap(args, [&](auto && arg) { return arg->clone_with_replacement(repl); });
            return make_ready_future<expression *>(make_variable_expression(make_struct_variable(this->shared_from_this(), std::move(arg_copies))).release());
        });

        _aggregate_ctor->set_name(U"constructor");

        _aggregate_ctor_promise->set(_aggregate_ctor.get());

        data_members = fmap(_data_members, [&](auto && member) -> variable * {
            auto param = make_member_variable(member->get_original(), member->get_name());
            auto expr = make_variable_ref_expression(member);
            param->set_default_value(expr.get());

            auto ret = param.get();
            _member_copy_arguments.push_back(std::move(param));
            _member_copy_defaults.push_back(std::move(expr));
            return ret;
        });

        _this_argument = make_blank_variable(this);
        data_members.insert(data_members.begin(), _this_argument.get());

        _aggregate_copy_ctor =
            make_function("struct type copy replacement constructor", get_expression(), data_members, [&](auto && ctx) -> codegen::ir::function {
                auto ir_type = this->codegen_type(ctx);
                auto args = fmap(_data_members, [&](auto && member) { return get<codegen::ir::value>(member->codegen_ir(ctx)); });
                auto result = codegen::ir::make_variable(ir_type);

                return { U"constructor",
                    this->get_scope()->codegen_ir(ctx),
                    fmap(args, [](auto && arg) { return get<std::shared_ptr<codegen::ir::variable>>(arg); }),
                    result,
                    { codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::aggregate_init_instruction>() }, args, result } } };
            });

        _aggregate_copy_ctor->set_eval([this](auto &&, std::vector<variable *> args) {
            [[maybe_unused]] auto base = args.front();
            args.erase(args.begin());

            if (base->get_type() != this || !std::equal(args.begin(), args.end(), _data_members.begin(), [](auto && arg, auto && member) {
                    return arg->get_type() == member->get_type();
                }))
            {
                assert(0);
            }

            auto repl = replacements{};
            for (std::size_t i = 0; i < _data_members.size(); ++i)
            {
                repl.variables[base->get_member(_data_members[i])] = args[i];
            }
            return make_ready_future(make_variable_expression(base->clone_with_replacement(repl)).release());
        });

        _aggregate_copy_ctor->set_name(U"replacing_copy_constructor");
        _aggregate_copy_ctor->make_member();

        _aggregate_copy_ctor_promise->set(_aggregate_copy_ctor.get());
    }

    void struct_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;

        // TODO: actual name tracking for this shit
        auto type = codegen::ir::variable_type{ U"__struct_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.struct_index++)),
            get_scope()->codegen_ir(ctx),
            0,
            fmap(_data_members, [&](auto && member) { return codegen::ir::member{ member->member_codegen_ir(ctx) }; }) };

        *actual_type = std::move(type);
    }
}
}
