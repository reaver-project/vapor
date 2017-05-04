/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include "../types/pack.h"
#include "variable.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<variable> make_pack_variable(std::vector<std::unique_ptr<variable>> vars, type * pack_type = builtin_types().type->get_pack_type());

    class pack_variable : public variable
    {
    public:
        pack_variable(std::vector<variable *> vars, type * pack_type) : _vars{ std::move(vars) }, _type{ pack_type }
        {
        }

        virtual type * get_type() const override
        {
            return _type;
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements & repl) const override
        {
            return make_pack_variable(fmap(_vars, [&](auto && var) { return var->clone_with_replacement(repl); }), _type);
        }

        virtual variable_ir _codegen_ir(ir_generation_context &) const override
        {
            assert(0);
        }

        std::vector<variable *> _vars;
        type * _type;
    };

    class owning_pack_variable : public pack_variable
    {
    public:
        owning_pack_variable(std::vector<std::unique_ptr<variable>> vars, type * pack_type)
            : pack_variable{ fmap(vars, [](auto && var) { return var.get(); }), pack_type }, _vars{ std::move(vars) }
        {
        }

    private:
        std::vector<std::unique_ptr<variable>> _vars;
    };

    inline auto make_pack_variable(std::vector<variable *> vars = {}, type * pack_type = builtin_types().type->get_pack_type())
    {
        return std::make_unique<pack_variable>(std::move(vars), pack_type);
    }

    inline std::unique_ptr<variable> make_pack_variable(std::vector<std::unique_ptr<variable>> vars, type * pack_type)
    {
        return std::make_unique<owning_pack_variable>(std::move(vars), pack_type);
    }
}
}
