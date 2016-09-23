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

#include "vapor/parser.h"
#include "vapor/analyzer/overload_set.h"
#include "vapor/analyzer/function.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/ir/type.h"

void reaver::vapor::analyzer::_v1::overload_set_type::add_function(std::shared_ptr<reaver::vapor::analyzer::_v1::function> fn)
{
    if (std::find_if(_functions.begin(), _functions.end(), [&](auto && f) {
            return f->arguments() == fn->arguments();
        }) != _functions.end())
    {
        assert(0);
    }

    _functions.push_back(std::move(fn));
}

std::shared_ptr<reaver::vapor::analyzer::_v1::function> reaver::vapor::analyzer::_v1::overload_set_type::get_overload(reaver::vapor::lexer::token_type bracket, std::vector<std::shared_ptr<reaver::vapor::analyzer::_v1::type>> args) const
{
    if (bracket == lexer::token_type::round_bracket_open)
    {
        auto it = std::find_if(_functions.begin(), _functions.end(), [&](auto && f) {
            return f->arguments() == args;
        });

        if (it != _functions.end())
        {
            return *it;
        }
    }

    return nullptr;
}

std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> reaver::vapor::analyzer::_v1::overload_set_type::_codegen_type(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto type = codegen::ir::make_type(
        U"__overload_set_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.overload_set_index++)),
        _scope->codegen_ir(ctx),
        0,
        fmap(_functions, [&](auto && fn) {
            ctx.add_generated_function(fn);
            return codegen::ir::member{ fn->codegen_ir(ctx) };
        })
    );

    auto scopes = _scope->codegen_ir(ctx);
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
    return { codegen::ir::make_variable(_type->codegen_type(ctx)) };
}

void reaver::vapor::analyzer::_v1::function_declaration::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "function declaration of `" << utf8(_parse.name.string) << "` at " << _parse.range << '\n';
    assert(!_parse.arguments);
    os << in << "return type: " << _function->return_type()->explain() << '\n';
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
    return _body->analyze().then([&]{
        _function = make_function(
            "overloadable function",
            _body->return_type(),
            {},
            [=, name = _parse.name.string](ir_generation_context & ctx) {
                return codegen::ir::function{
                    U"operator()",
                    {}, {},
                    _body->codegen_return(ctx),
                    _body->codegen_ir(ctx)
                };
            },
            _parse.range
        );

        auto set = _scope->get(_parse.name.string);
        auto overloads = std::dynamic_pointer_cast<overload_set>(set->get_variable());
        assert(overloads);
        overloads->add_function(shared_from_this());
    });
}

