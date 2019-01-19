/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/expressions/expression_ref.h"
#include "vapor/analyzer/expressions/overload_set.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/semantic/function.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/codegen/ir/function.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/parser.h"

#include "expressions/overload_set.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<expression> overload_set::_clone_expr_with_replacement(replacements & repl) const
    {
        // icky
        return make_expression_ref(const_cast<overload_set *>(this), get_ast_info());
    }

    statement_ir overload_set::_codegen_ir(ir_generation_context & ctx) const
    {
        auto var = codegen::ir::make_variable(_type->codegen_type(ctx));
        var->scopes = _type->get_scope()->codegen_ir();
        return { codegen::ir::instruction{
            std::nullopt, std::nullopt, { boost::typeindex::type_id<codegen::ir::pass_value_instruction>() }, {}, codegen::ir::value{ std::move(var) } } };
    }

    declaration_ir overload_set::declaration_codegen_ir(ir_generation_context & ctx) const
    {
        return { { std::get<std::shared_ptr<codegen::ir::variable>>(codegen_ir(ctx).back().result) } };
    }

    void overload_set::add_function(function_declaration * decl)
    {
        _function_decls.push_back(decl);
        _type->add_function(decl->get_function());
    }

    std::unique_ptr<google::protobuf::Message> overload_set::_generate_interface() const
    {
        return std::make_unique<proto::overload_set>();
    }

    std::shared_ptr<overload_set> create_overload_set(scope * lex_scope, std::u32string name)
    {
        auto type_name = U"overload_set_type$" + name;

        auto oset = std::make_shared<overload_set>(lex_scope);
        lex_scope->init(name, make_symbol(name, oset.get()));

        auto type = oset->get_type();
        type->set_name(type_name);
        lex_scope->init(type_name, make_symbol(type_name, type->get_expression()));

        return oset;
    }

    std::shared_ptr<overload_set> get_overload_set(scope * lex_scope, std::u32string name)
    {
        auto symbol = lex_scope->try_get(name);

        if (!symbol)
        {
            return create_overload_set(lex_scope, name);
        }

        return symbol.value()->get_expression()->as<overload_set>()->shared_from_this();
    }
}
}
