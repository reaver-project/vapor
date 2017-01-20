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

#pragma once

#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class member_variable : public variable
    {
    public:
        member_variable(variable * original, std::u32string name) : _wrapped{ original }, _name{ std::move(name) }
        {
        }

        virtual type * get_type() const override
        {
            return _wrapped->get_type();
        }

        virtual bool is_member() const override
        {
            return true;
        }

        codegen::ir::member_variable member_codegen_ir(ir_generation_context & ctx) const;

        const std::u32string & get_name() const
        {
            return _name;
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements & repl) const override
        {
            assert(0);
        }

        virtual variable_ir _codegen_ir(ir_generation_context & ctx) const override
        {
            return _wrapped->codegen_ir(ctx);
        }

        variable * _wrapped = nullptr;
        std::u32string _name;
    };

    inline auto make_member_variable(variable * original, std::u32string name)
    {
        return std::make_unique<member_variable>(original, std::move(name));
    }
}
}
