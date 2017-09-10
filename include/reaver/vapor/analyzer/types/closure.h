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
#include <numeric>

#include "../expressions/closure.h"
#include "../function.h"
#include "type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class closure_type : public type
    {
    public:
        closure_type(scope * lex_scope, class closure * closure, std::unique_ptr<function> fn)
            : type{ lex_scope }, _closure{ std::move(closure) }, _function{ std::move(fn) }
        {
        }

        virtual std::string explain() const override
        {
            return "closure (TODO: location)";
        }

        virtual void print(std::ostream & os, print_context ctx) const override;

        virtual future<std::vector<function *>> get_candidates(lexer::token_type bracket) const override;

        const auto & parse() const
        {
            return _closure->parse();
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        virtual std::u32string _codegen_name(ir_generation_context & ctx) const override
        {
            if (!_codegen_type_name)
            {
                _codegen_type_name = U"closure_" + utf32(std::to_string(ctx.closure_index++)), get_scope()->codegen_ir(ctx);
            }

            return *_codegen_type_name;
        }

        closure * _closure;
        std::unique_ptr<function> _function;
        mutable optional<std::u32string> _codegen_type_name;
    };
}
}
