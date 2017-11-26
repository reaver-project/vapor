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

#include <reaver/overloads.h>
#include <reaver/variant.h>

#include "generator.h"

#include "ir/instruction.h" // TODO: remove

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    class llvm_ir_generator : public code_generator
    {
    public:
        virtual std::u32string generate_global_definitions(codegen_context &) const override;
        virtual std::u32string generate_definitions(ir::module &, codegen_context &) override;
        virtual std::u32string generate_definition(std::shared_ptr<ir::variable_type>, codegen_context &) override;

        std::u32string generate_definition(ir::variable &, codegen_context &);
        std::u32string generate_definition(ir::function &, codegen_context &);

        // TODO: remove definition
        template<typename T>
        static std::u32string generate(const ir::instruction & inst, codegen_context & ctx)
        {
            return U"needed: generate<" + utf32(inst.instruction.explain()) + U">\n";
        }

    private:
        static std::u32string type_name(std::shared_ptr<ir::variable_type>, codegen_context &);
        static std::u32string function_name(ir::function &, codegen_context &);
        static std::u32string variable_name(ir::variable &, codegen_context &);

        static std::u32string variable_of(const ir::value & val, codegen_context & ctx)
        {
            assert(val.index() == 0);
            return variable_name(*std::get<std::shared_ptr<ir::variable>>(val), ctx);
        }

        static std::u32string type_of(const ir::value & val, codegen_context & ctx)
        {
            return std::get<std::u32string>(fmap(val,
                make_overload_set(
                    [&](const ir::integer_value & val) {
                        assert(val.size);

                        std::ostringstream os;
                        os << "i" << val.size.value();
                        return utf32(os.str());
                    },
                    [&](const ir::boolean_value &) -> std::u32string { return U"i1"; },
                    [&](const std::shared_ptr<ir::variable> & var) {
                        ctx.put_into_global_before += ctx.define_if_necessary(var->type);
                        return type_name(var->type, ctx);
                    },
                    [&](const ir::label &) {
                        assert(0);
                        return unit{};
                    },
                    [&](const ir::struct_value & val) {
                        ctx.put_into_global_before += ctx.define_if_necessary(val.type);
                        return type_name(val.type, ctx);
                    },
                    [](auto &&) {
                        assert(0);
                        return unit{};
                    })));
        }

        static std::u32string value_of(const ir::value & val, codegen_context & ctx)
        {
            return std::get<std::u32string>(fmap(val,
                make_overload_set(
                    [&](const codegen::ir::integer_value & val) {
                        std::ostringstream os;
                        os << val.value;
                        return utf32(os.str());
                    },
                    [&](const codegen::ir::boolean_value & val) -> std::u32string { return val.value ? U"true" : U"false"; },
                    [&](const std::shared_ptr<ir::variable> & var) { return variable_name(*var, ctx); },
                    [&](const codegen::ir::label & label) {
                        assert(label.scopes.empty());
                        return U"%\"" + label.name + U"\"";
                    },
                    [&](const codegen::ir::struct_value & val) {
                        std::u32string ret;
                        ret += U"{";

                        for (auto && member : val.fields)
                        {
                            ret += U" " + type_of(member, ctx) + U" " + value_of(member, ctx) + U",";
                        }

                        if (ret.back() == U',')
                        {
                            ret.pop_back();
                        }
                        ret += U" }";

                        return ret;
                    },
                    [&](auto &&) {
                        assert(0);
                        return unit{};
                    })));
        }
        std::u32string generate(const ir::instruction &, codegen_context &);
    };

    inline std::shared_ptr<code_generator> make_llvm_ir()
    {
        return std::make_shared<llvm_ir_generator>();
    }
}
}
