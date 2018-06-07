/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2018 Michał "Griwes" Dominiak
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
    // this is a little awkward, but a typeclass *is* a type for its instances...
    class typeclass : public user_defined_type
    {
    public:
        typeclass() = default;

        virtual std::string explain() const override
        {
            return "a typeclass (TODO: add name tracking to this stuff)";
        }

        virtual void print(std::ostream & os, print_context ctx) const override;
        virtual future<std::vector<function *>> get_candidates(lexer::token_type bracket) const override;

    private:
        virtual std::unique_ptr<google::protobuf::Message> _user_defined_interface() const override;

        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual std::u32string _codegen_name(ir_generation_context &) const override
        {
            assert(0);
        }
    };

    std::unique_ptr<type> make_typeclass_type();
}
}
