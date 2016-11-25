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
#include "vapor/analyzer/statements/overload_set.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void overload_set_type::add_function(function * fn)
    {
        std::unique_lock<std::mutex> lock{ _functions_lock };

        if (std::find_if(_functions.begin(), _functions.end(), [&](auto && f) { return f->arguments() == fn->arguments(); }) != _functions.end())
        {
            assert(0);
        }

        _functions.push_back(fn);
    }

    future<function *> overload_set_type::get_overload(lexer::token_type bracket, std::vector<const type *> args) const
    {
        std::unique_lock<std::mutex> lock{ _functions_lock };

        if (bracket == lexer::token_type::round_bracket_open)
        {
            auto it = std::find_if(_functions.begin(), _functions.end(), [&](auto && f) {
                // this is dumb
                // but you apparently can't compare `vector<T>` and `vector<const T>`...
                return args.size() == f->arguments().size()
                    && std::inner_product(args.begin(), args.end(), f->arguments().begin(), true, std::logical_and<>(), [](auto && type, auto && var) {
                           return type == var->get_type();
                       });
            });

            if (it != _functions.end())
            {
                auto ret = *it;
                return make_ready_future(ret);
            }
        }

        assert(0);
        return make_ready_future(static_cast<function *>(nullptr));
    }

    void overload_set_type::_codegen_type(ir_generation_context & ctx) const
    {
        auto actual_type = *_codegen_t;

        auto type = codegen::ir::variable_type{ U"__overload_set_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.overload_set_index++)),
            get_scope()->codegen_ir(ctx),
            0,
            fmap(_functions, [&](auto && fn) {
                ctx.add_generated_function(fn);
                return codegen::ir::member{ fn->codegen_ir(ctx) };
            }) };

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

    std::unique_ptr<variable> overload_set::_clone_with_replacement(replacements & repl) const
    {
        assert(0);
    }

    variable_ir overload_set::_codegen_ir(ir_generation_context & ctx) const
    {
        auto var = codegen::ir::make_variable(_type->codegen_type(ctx));
        var->scopes = _type->get_scope()->codegen_ir(ctx);
        return { std::move(var) };
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
        os << in << "return type: " << (*_function->return_type().try_get())->explain() << '\n';
        os << in << "{\n";
        _body->print(os, indent + 4);
        os << in << "}\n";
    }

    statement_ir function_declaration::_codegen_ir(ir_generation_context &) const
    {
        return {};
    }

    future<> function_declaration::_analyze(analysis_context & ctx)
    {
        _function = make_function("overloadable function",
            nullptr,
            {},
            [=, name = _parse.name.string](ir_generation_context & ctx) {
                return codegen::ir::function{ U"operator()",
                    {},
                    fmap(_argument_list,
                        [&](auto && arg) { return get<std::shared_ptr<codegen::ir::variable>>(get<codegen::ir::value>(arg.variable->codegen_ir(ctx))); }),
                    _body->codegen_return(ctx),
                    _body->codegen_ir(ctx) };
            },
            _parse.range);
        _function->set_name(U"operator()");
        _overload_set->add_function(this);

        auto initial_future = [&] {
            if (_return_type)
            {
                return (*_return_type)->analyze(ctx).then([&] {
                    auto var = (*_return_type)->get_variable();

                    assert(var->get_type() == builtin_types().type.get());
                    assert(var->is_constant());

                    _function->set_return_type(dynamic_cast<type_variable *>(var)->get_value());
                });
            }

            return make_ready_future();
        }();

        return initial_future.then([&] { return when_all(fmap(_argument_list, [&](auto && arg) { return arg.type_expression->analyze(ctx); })); })
            .then([&] {
                auto arg_variables = fmap(_argument_list, [&](auto && arg) -> variable * {
                    arg.variable->set_type(arg.type_expression->get_variable());
                    return arg.variable.get();
                });

                _function->set_arguments(std::move(arg_variables));

                return _body->analyze(ctx);
            })
            .then([&] {
                fmap(_return_type, [&](auto && ret_type) {
                    auto explicit_type = ret_type->get_variable();
                    auto type_var = static_cast<type_variable *>(explicit_type);

                    assert(type_var->get_value() == _body->return_type());

                    return unit{};
                });

                _function->set_body(_body.get());

                if (!_function->return_type().try_get())
                {
                    _function->set_return_type(_body->return_type());
                }
            });
    }

    future<statement *> function_declaration::_simplify(simplification_context & ctx)
    {
        return _body->simplify(ctx).then([&](auto && simplified) -> statement * {
            replace_uptr(_body, dynamic_cast<block *>(simplified), ctx);
            return this;
        });
    }
}
}
