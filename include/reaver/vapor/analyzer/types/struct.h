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

#include "../function.h"
#include "../statements/statement.h"
#include "type.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct struct_literal;
}
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class declaration;

    class struct_type : public type
    {
    public:
        struct_type(const parser::struct_literal & parse, scope * lex_scope);
        ~struct_type();

        virtual future<function *> get_constructor(std::vector<const variable *> args) const override;
        void generate_constructor();

        virtual std::string explain() const override
        {
            return "struct type (TODO: trace information!)";
        }

        std::vector<declaration *> get_data_member_decls() const
        {
            return fmap(_data_members_declarations, [](auto && ptr) { return ptr.get(); });
        }

        std::vector<const variable *> get_data_members() const
        {
            return fmap(_data_members, [](auto && ptr) -> const variable * { return ptr; });
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        const parser::struct_literal & _parse;

        std::vector<std::unique_ptr<declaration>> _data_members_declarations;
        std::vector<variable *> _data_members;

        std::unique_ptr<function> _aggregate_ctor;
        mutable optional<future<function *>> _aggregate_ctor_future;
        optional<manual_promise<function *>> _aggregate_ctor_promise;
    };

    inline auto make_struct_type(const parser::struct_literal & parse, scope * lex_scope)
    {
        return std::make_unique<struct_type>(parse, lex_scope);
    }
}
}
