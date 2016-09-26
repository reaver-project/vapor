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

#include "vapor/analyzer/closure.h"
#include "vapor/codegen/ir/type.h"

std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable_type> reaver::vapor::analyzer::_v1::closure_type::_codegen_type(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto type = codegen::ir::make_type(U"__closure_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.closure_index++)), get_scope()->codegen_ir(ctx), 0, {});

    auto scopes = get_scope()->codegen_ir(ctx);
    scopes.emplace_back(type->name, codegen::ir::scope_type::type);

    auto fn = _function->codegen_ir(ctx);
    fn.scopes = scopes;
    fn.parent_type = type;
    type->members = { codegen::ir::member{ fn } };

    ctx.add_generated_function(_function);

    return type;
}

void reaver::vapor::analyzer::_v1::closure::print(std::ostream & os, std::size_t indent) const
{
    auto in = std::string(indent, ' ');
    os << in << "closure at " << _parse.range << '\n';
    assert(!_parse.captures);
    assert(!_parse.arguments);
    os << in << "return type: " << _body->return_type()->explain() << '\n';
    os << in << "{\n";
    _body->print(os, indent + 4);
    os << in << "}\n";
}

reaver::future<> reaver::vapor::analyzer::_v1::closure::_analyze()
{
    return _body->analyze().then([&]
    {
        auto function = make_function(
            "closure",
            _body->return_type(),
            {},
            [this](ir_generation_context & ctx) {
                return codegen::ir::function{
                    U"operator()",
                    {}, {},
                    _body->codegen_return(ctx),
                    _body->codegen_ir(ctx),
                };
            },
            _parse.range
        );
        _type = std::make_shared<closure_type>(_scope, _weak_from_this(), std::move(function));
        _set_variable(make_expression_variable(_weak_from_this(), _type));
    });
}

reaver::future<std::shared_ptr<reaver::vapor::analyzer::_v1::expression>> reaver::vapor::analyzer::_v1::closure::_simplify_expr(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    return _body->simplify(ctx)
        .then([&](auto && simplified) {
            _body = std::dynamic_pointer_cast<block>(std::move(simplified));
            assert(0);
            return _shared_from_this();
        });
}

reaver::vapor::analyzer::_v1::statement_ir reaver::vapor::analyzer::_v1::closure::_codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    auto var = codegen::ir::make_variable(get_variable()->get_type()->codegen_type(ctx));
    var->scopes = _type->get_scope()->codegen_ir(ctx);
    return {
        codegen::ir::instruction{
            none, none,
            { boost::typeindex::type_id<codegen::ir::materialization_instruction>() },
            {},
            { std::move(var) }
        }
    };
}

