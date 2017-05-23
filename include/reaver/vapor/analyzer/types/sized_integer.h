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

#include "type.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class sized_integer : public type
    {
    public:
        sized_integer(std::size_t size);

        virtual std::string explain() const override
        {
            return "sized_integer(" + std::to_string(_size) + ")";
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            assert(0);
        }

        virtual future<std::vector<function *>> get_candidates(lexer::token_type token) const override
        {
            auto ret = [](auto && fn) { return make_ready_future(std::vector<function *>{ fn.get() }); };

            switch (token)
            {
                case lexer::token_type::plus:
                    return ret(_addition);

                case lexer::token_type::minus:
                    return ret(_subtraction);

                case lexer::token_type::star:
                    return ret(_multiplication);

                case lexer::token_type::equals:
                    return ret(_equal_comparison);

                case lexer::token_type::less:
                    return ret(_less_comparison);

                case lexer::token_type::less_equal:
                    return ret(_less_equal_comparison);

                default:
                    assert(!"unimplemented int op");
            }
        }

    private:
        virtual void _codegen_type(ir_generation_context & ctx) const override;

        std::size_t _size;

        template<typename Instruction, typename Eval>
        auto _generate_function(const char32_t * name, std::string desc, Eval eval, type * return_type);
        std::unique_ptr<function> _addition;
        std::unique_ptr<function> _subtraction;
        std::unique_ptr<function> _multiplication;
        std::unique_ptr<function> _equal_comparison;
        std::unique_ptr<function> _less_comparison;
        std::unique_ptr<function> _less_equal_comparison;
    };

    inline std::unique_ptr<sized_integer> make_sized_integer_type(std::size_t size)
    {
        return std::make_unique<sized_integer>(size);
    }
}
}
