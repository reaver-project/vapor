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

#include "vapor/analyzer/types/typeclass.h"
#include "vapor/analyzer/symbol.h"

#include "expressions/type.pb.h"
#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    void typeclass::print(std::ostream &, print_context) const
    {
        assert(0);
    }

    future<std::vector<function *>> typeclass::get_candidates(lexer::token_type) const
    {
        assert(0);
    }

    std::unique_ptr<google::protobuf::Message> typeclass::_user_defined_interface() const
    {
        assert(0);
    }

    // this is the type called `typeclass`
    // I need to think about exposing this, but obviously the keyword `typeclass` is not the way to go here
    // `typeof()` will get us there, but I'm not sure if that's how I want to do this either
    // FIGURE ME OUT: do I actually need to expose this one in a named way? is there a use case?
    class typeclass_type : public type
    {
    public:
        typeclass_type() : type{ dont_init_expr }
        {
        }

        virtual std::string explain() const override
        {
            return "typeclass";
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::type << "typeclass" << styles::def << " @ " << styles::address << this << styles::def << ": builtin type\n";
        }

        virtual std::unique_ptr<proto::type> generate_interface() const override
        {
            auto ret = std::make_unique<proto::type>();
            ret->set_allocated_reference(generate_interface_reference().release());
            return ret;
        }

        virtual std::unique_ptr<proto::type_reference> generate_interface_reference() const override
        {
            auto ret = std::make_unique<proto::type_reference>();
            ret->set_builtin(proto::typeclass);
            return ret;
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual std::u32string _codegen_name(ir_generation_context & ctx) const override
        {
            return U"typeclass";
        }
    };

    std::unique_ptr<type> make_typeclass_type()
    {
        return std::make_unique<typeclass_type>();
    }
}
}
