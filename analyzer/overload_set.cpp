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

#include "vapor/parser.h"
#include "vapor/analyzer/overload_set.h"
#include "vapor/analyzer/function.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/ir/type.h"

void reaver::vapor::analyzer::_v1::overload_set_type::add_function(reaver::vapor::analyzer::_v1::function * fn)
{
    std::unique_lock<std::mutex> lock{ _functions_lock };

    if (std::find_if(_functions.begin(), _functions.end(), [&](auto && f) {
            return f->arguments() == fn->arguments();
        }) != _functions.end())
    {
        assert(0);
    }

    _functions.push_back(fn);
}

reaver::future<reaver::vapor::analyzer::_v1::function *> reaver::vapor::analyzer::_v1::overload_set_type::get_overload(reaver::vapor::lexer::token_type bracket, std::vector<const reaver::vapor::analyzer::_v1::type *> args) const
{
    std::unique_lock<std::mutex> lock{ _functions_lock };

    if (bracket == lexer::token_type::round_bracket_open)
    {
        auto it = std::find_if(_functions.begin(), _functions.end(), [&](auto && f) {
            // this is dumb
            // but you apparently can't compare `vector<T>` and `vector<const T>`...
            return args.size() == f->arguments().size()
                && std::inner_product(
                    args.begin(), args.end(),
                    f->arguments().begin(),
                    true,
                    std::logical_and<>(),
                    std::equal_to<>()
                );
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

std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> reaver::vapor::analyzer::_v1::overload_set_type::_codegen_type(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto type = codegen::ir::make_type(
        U"__overload_set_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.overload_set_index++)),
        get_scope()->codegen_ir(ctx),
        0,
        fmap(_functions, [&](auto && fn) {
            ctx.add_generated_function(fn);
            return codegen::ir::member{ fn->codegen_ir(ctx) };
        })
    );

    auto scopes = get_scope()->codegen_ir(ctx);
    scopes.emplace_back(type->name, codegen::ir::scope_type::type);

    fmap(type->members, [&](auto && member) {
        fmap(member, make_overload_set(
            [&](codegen::ir::function & fn) {
                fn.scopes = scopes;
                fn.parent_type = type;

                return unit{};
            },
            [&](auto &&) {
                assert(0);
                return unit{};
            }
        ));
        return unit{};
    });

    return type;
}

reaver::vapor::analyzer::_v1::variable_ir reaver::vapor::analyzer::_v1::overload_set::_codegen_ir(ir_generation_context & ctx) const
{
    auto var = codegen::ir::make_variable(_type->codegen_type(ctx));
    var->scopes = _type->get_scope()->codegen_ir(ctx);
    return { std::move(var) };
}

void reaver::vapor::analyzer::_v1::function_declaration::print(std::ostream & os, std::size_t indent) const
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

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::function_declaration::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context &) const
{
    return {};
}

reaver::future<> reaver::vapor::analyzer::_v1::function_declaration::_analyze()
{
    _function = make_function(
        "overloadable function",
        nullptr,
        {},
        [=, name = _parse.name.string](ir_generation_context & ctx) {
            return codegen::ir::function{
                U"operator()",
                {},
                fmap(_argument_list, [&](auto && arg) {
                    return get<std::shared_ptr<codegen::ir::variable>>(
                        get<codegen::ir::value>(arg.variable->codegen_ir(ctx))
                    );
                }),
                _body->codegen_return(ctx),
                _body->codegen_ir(ctx)
            };
        },
        _parse.range
    );
    _overload_set->add_function(this);

    return when_all(fmap(_argument_list, [&](auto && arg) {
        return arg.type_expression->analyze();
    })).then([&]{
        auto arg_types = fmap(_argument_list, [&](auto && arg) {
            arg.variable->set_type(arg.type_expression->get_variable());

            // TODO: this must be done somewhat differently
            auto type_var = dynamic_cast<type_variable *>(arg.type_expression->get_variable());
            assert(type_var);
            return type_var->get_value();
        });

        _function->set_arguments(std::move(arg_types));

        return _body->analyze();
    }).then([&]{
        _function->set_body(_body.get());
        _function->set_return_type(_body->return_type());
    });
}

reaver::future<reaver::vapor::analyzer::_v1::statement *> reaver::vapor::analyzer::_v1::function_declaration::_simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    return _body->simplify(ctx).then([&](auto && simplified) -> statement * {
        replace_uptr(_body, dynamic_cast<block *>(simplified), ctx);
        return this;
    });
}

