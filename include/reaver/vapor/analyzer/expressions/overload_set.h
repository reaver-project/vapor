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

#pragma once

#include <memory>

#include "../semantic/overload_set.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function_declaration;
    class function_definition;
    class overload_set;

    class overload_set_expression : public expression
    {
    public:
        overload_set_expression(scope * lex_scope) : _oset{ std::make_unique<overload_set>(lex_scope) }
        {
            _set_type(_oset->get_type());
        }

        overload_set_expression(std::shared_ptr<overload_set> t) : _oset{ std::move(t) }
        {
            _set_type(_oset->get_type());
        }

        const std::vector<function *> & get_overloads() const
        {
            return _oset->get_overloads();
        }

        virtual void print(std::ostream & os, print_context) const override
        {
            assert(0);
        }

        virtual declaration_ir declaration_codegen_ir(ir_generation_context & ctx) const override;

        virtual void mark_exported() override
        {
            _oset->get_type()->mark_exported();
        }

        std::shared_ptr<overload_set> get_overload_set() const
        {
            return _oset;
        }

    private:
        virtual std::unique_ptr<expression> _clone_expr(replacements &) const override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;
        virtual std::unique_ptr<google::protobuf::Message> _generate_interface() const override;

        std::shared_ptr<overload_set> _oset;
    };

    std::unique_ptr<overload_set_expression> create_overload_set(scope * lex_scope, std::u32string name);
    std::unique_ptr<overload_set_expression> get_overload_set(scope * lex_scope, std::u32string name);
}
}
