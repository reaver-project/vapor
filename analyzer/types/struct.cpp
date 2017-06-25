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
#include "vapor/analyzer/expressions/blank.h"
#include "vapor/analyzer/expressions/struct.h"
#include "vapor/analyzer/expressions/struct_value.h"
#include "vapor/analyzer/statements/declaration.h"
#include "vapor/analyzer/symbol.h"
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
        _data_members = fmap(_data_members_declarations, [&](auto && member) { return member->declared_member(); });

        _aggregate_ctor = make_function("struct type constructor",
            get_expression(),
            fmap(_data_members, [](auto && member) -> expression * { return member; }),
            [&](auto && ctx) -> codegen::ir::function {
                auto ir_type = this->codegen_type(ctx);
                auto args = fmap(_data_members, [&](auto && member) { return member->codegen_ir(ctx).back().result; });
                auto result = codegen::ir::make_variable(ir_type);

                auto scopes = this->get_scope()->codegen_ir(ctx);
                scopes.emplace_back(_codegen_type_name_value.get(), codegen::ir::scope_type::type);

                return { U"constructor",
                    std::move(scopes),
                    fmap(args, [](auto && arg) { return get<std::shared_ptr<codegen::ir::variable>>(arg); }),
                    result,
                    { codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::aggregate_init_instruction>() }, args, result },
                        codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::return_instruction>() }, { result }, result } } };
            });

        _aggregate_ctor->set_scopes_generator([this](auto && ctx) {
            this->codegen_type(ctx);

            auto scopes = this->get_scope()->codegen_ir(ctx);
            scopes.emplace_back(_codegen_type_name_value.get(), codegen::ir::scope_type::type);

            return scopes;
        });

        _aggregate_ctor->set_eval([this](auto &&, const std::vector<expression *> & args) {
            if (!std::equal(args.begin(), args.end(), _data_members.begin(), [](auto && arg, auto && member) { return arg->get_type() == member->get_type(); }))
            {
                assert(0);
            }

            if (!std::all_of(args.begin(), args.end(), [](auto && arg) { return arg->is_constant(); }))
            {
                return make_ready_future<expression *>(nullptr);
            }

            auto repl = replacements{};
            auto arg_copies = fmap(args, [&](auto && arg) { return arg->clone_expr_with_replacement(repl); });
            return make_ready_future<expression *>(make_struct_expression(this->shared_from_this(), std::move(arg_copies)).release());
        });

        _aggregate_ctor->set_name(U"constructor");

        _aggregate_ctor_promise->set(_aggregate_ctor.get());

        auto data_members = fmap(_data_members, [&](auto && member) -> expression * {
            /*auto param = make_member_variable(member->get_original(), member->get_name());
            auto expr = make_variable_ref_expression(member);
            param->set_default_value(expr.get());

            auto ret = param.get();
            _member_copy_arguments.push_back(std::move(param));
            _member_copy_defaults.push_back(std::move(expr));
            return ret;*/

            assert(0); // need to clone everything to give alternative default values
        });

        _this_argument = make_blank_expression(this);
        data_members.insert(data_members.begin(), _this_argument.get());

        _aggregate_copy_ctor =
            make_function("struct type copy replacement constructor", get_expression(), data_members, [&](auto && ctx) -> codegen::ir::function {
                auto ir_type = this->codegen_type(ctx);
                auto args = fmap(_data_members, [&](auto && member) { return member->codegen_ir(ctx).back().result; });
                auto result = codegen::ir::make_variable(ir_type);

                auto scopes = this->get_scope()->codegen_ir(ctx);
                scopes.emplace_back(_codegen_type_name_value.get(), codegen::ir::scope_type::type);

                return { U"replacing_copy_constructor",
                    std::move(scopes),
                    fmap(args, [](auto && arg) { return get<std::shared_ptr<codegen::ir::variable>>(arg); }),
                    result,
                    { codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::aggregate_init_instruction>() }, args, result },
                        codegen::ir::instruction{ none, none, { boost::typeindex::type_id<codegen::ir::return_instruction>() }, { result }, result } } };
            });

        _aggregate_copy_ctor->set_eval([this](auto &&, std::vector<expression *> args) {
            auto base = args.front();
            args.erase(args.begin());

            if (base->get_type() != this || !std::equal(args.begin(), args.end(), _data_members.begin(), [](auto && arg, auto && member) {
                    return arg->get_type() == member->get_type();
                }))
            {
                assert(0);
            }

            if (!std::all_of(args.begin(), args.end(), [](auto && arg) { return arg->is_constant(); }))
            {
                return make_ready_future<expression *>(nullptr);
            }

            auto repl = replacements{};
            for (std::size_t i = 0; i < _data_members.size(); ++i)
            {
                repl.expressions[base->get_member(_data_members[i]->get_name())] = args[i];
            }
            return make_ready_future(base->clone_expr_with_replacement(repl).release());
        });

        _aggregate_copy_ctor->set_name(U"replacing_copy_constructor");
        _aggregate_copy_ctor->make_member();

        _aggregate_copy_ctor_promise->set(_aggregate_copy_ctor.get());
    }

    void struct_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;

        // TODO: actual name tracking for this shit
        _codegen_type_name_value = U"__struct_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.struct_index++));
        auto type = codegen::ir::variable_type{ _codegen_type_name_value.get(), get_scope()->codegen_ir(ctx), 0, {} };

        auto members = fmap(_data_members, [&](auto && member) { return codegen::ir::member{ member->member_codegen_ir(ctx) }; });

        auto scopes = this->get_scope()->codegen_ir(ctx);
        scopes.emplace_back(type.name, codegen::ir::scope_type::type);

        auto add_fn = [&](auto && fn) {
            auto fn_ir = fn->codegen_ir(ctx);
            fn_ir.scopes = scopes;
            fn_ir.parent_type = actual_type;
            members.push_back(codegen::ir::member{ fn_ir });
            ctx.add_generated_function(fn.get());
        };
        add_fn(_aggregate_ctor);
        add_fn(_aggregate_copy_ctor);

        type.members = std::move(members);

        *actual_type = std::move(type);
    }

    void struct_type::print(std::ostream & os, print_context ctx) const
    {
        os << styles::def << ctx << styles::type << "struct type";
        print_address_range(os, this);
        os << '\n';
    }
}
}
