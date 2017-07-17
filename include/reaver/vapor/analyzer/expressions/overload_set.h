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

#include "../types/overload_set.h"
#include "expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class function_declaration;

    class overload_set : public expression, public std::enable_shared_from_this<overload_set>
    {
    public:
        overload_set(scope * lex_scope) : _type{ std::make_unique<overload_set_type>(lex_scope) }
        {
            _set_type(_type.get());
        }

        void add_function(function_declaration * fn);

        virtual void print(std::ostream & os, print_context) const override
        {
            assert(0);
        }

        virtual declaration_ir declaration_codegen_ir(ir_generation_context & ctx) const override;

    private:
        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override;
        virtual statement_ir _codegen_ir(ir_generation_context &) const override;

        std::vector<function_declaration *> _function_decls;
        std::unique_ptr<overload_set_type> _type;
    };
}
}
