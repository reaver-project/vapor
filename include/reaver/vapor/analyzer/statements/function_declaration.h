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

#pragma once

#include <memory>

#include "../../parser/function.h"
#include "../expressions/expression.h"
#include "../function.h"
#include "../semantic/parameter_list.h"
#include "../statements/block.h"
#include "../statements/statement.h"
#include "../symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function;
    class overload_set;

    class function_declaration : public statement
    {
    public:
        function_declaration(const parser::function & parse, scope * parent_scope);

        function * get_function() const
        {
            return _function.get();
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        const auto & parse() const
        {
            return _parse;
        }

    private:
        virtual future<> _analyze(analysis_context &) override;

        virtual std::unique_ptr<statement> _clone_with_replacement(replacements &) const override
        {
            return make_null_statement();
        }

        virtual future<statement *> _simplify(recursive_context) override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        const parser::function & _parse;
        parameter_list _parameter_list;

        optional<std::unique_ptr<expression>> _return_type;
        std::unique_ptr<block> _body;
        std::unique_ptr<scope> _scope;
        std::unique_ptr<function> _function;
        std::shared_ptr<overload_set> _overload_set;
        std::unique_ptr<expression> _self;
    };

    inline std::unique_ptr<function_declaration> preanalyze_function(const parser::function & func, scope *& lex_scope)
    {
        return std::make_unique<function_declaration>(func, lex_scope);
    }
}
}
