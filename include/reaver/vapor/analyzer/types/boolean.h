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

#include <memory>

#include "../../lexer/token.h"
#include "../function.h"
#include "type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class boolean_type : public type
    {
    public:
        virtual future<function *> get_overload(lexer::token_type token, const type * rhs) const override
        {
            switch (token)
            {
                case lexer::token_type::equals:
                    return make_ready_future(_equal_comparison());

                default:
                    assert(!"unimplemented int op");
            }
        }

        virtual std::string explain() const override
        {
            return "boolean";
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override;

        template<typename Instruction, typename Eval>
        static auto _generate_function(const char32_t * name, const char * desc, Eval eval, type * return_type);
        static function * _equal_comparison();
    };

    std::unique_ptr<type> make_boolean_type();
}
}
