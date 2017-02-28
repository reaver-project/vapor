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

#include "../function.h"
#include "../statements/statement.h"
#include "../variables/member.h"
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

    class struct_type : public type, public std::enable_shared_from_this<struct_type>
    {
    public:
        struct_type(const parser::struct_literal & parse, scope * lex_scope);
        ~struct_type();

        void generate_constructor();

        virtual std::string explain() const override
        {
            return "struct type (TODO: trace information!)";
        }

        std::vector<declaration *> get_data_member_decls() const
        {
            return fmap(_data_members_declarations, [](auto && ptr) { return ptr.get(); });
        }

        const std::vector<member_variable *> & get_data_members() const
        {
            return _data_members;
        }

        virtual future<std::vector<function *>> get_candidates(lexer::token_type op) const override
        {
            if (op != lexer::token_type::curly_bracket_open)
            {
                return make_ready_future(std::vector<function *>{});
            }

            return make_ready_future(std::vector<function *>{ _aggregate_ctor.get() });
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        const parser::struct_literal & _parse;

        std::vector<std::unique_ptr<declaration>> _data_members_declarations;
        std::vector<member_variable *> _data_members;

        std::unique_ptr<function> _aggregate_ctor;
        mutable optional<future<function *>> _aggregate_ctor_future;
        optional<manual_promise<function *>> _aggregate_ctor_promise;

        mutable optional<std::u32string> _codegen_type_name_value;

        std::u32string _codegen_type_name(ir_generation_context & ctx) const
        {
            if (!_codegen_type_name_value)
            {
                _codegen_type_name_value = U"__struct_" + boost::locale::conv::utf_to_utf<char32_t>(std::to_string(ctx.struct_index++));
            }

            return *_codegen_type_name_value;
        }
    };

    inline auto make_struct_type(const parser::struct_literal & parse, scope * lex_scope)
    {
        return std::make_unique<struct_type>(parse, lex_scope);
    }
}
}
