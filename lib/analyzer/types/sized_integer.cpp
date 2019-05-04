/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/types/sized_integer.h"
#include "vapor/analyzer/expressions/boolean.h"
#include "vapor/analyzer/expressions/runtime_value.h"
#include "vapor/analyzer/expressions/sized_integer.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/types/boolean.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/ir/variable.h"

#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    std::unique_ptr<proto::type> sized_integer::generate_interface() const
    {
        auto sized = std::make_unique<proto::sized_integer>();
        sized->set_size(_size);

        auto type = std::make_unique<proto::type>();
        type->set_allocated_reference(generate_interface_reference().release());
        return type;
    }

    std::unique_ptr<proto::type_reference> sized_integer::generate_interface_reference() const
    {
        auto sized = std::make_unique<proto::sized_integer>();
        sized->set_size(_size);

        auto type = std::make_unique<proto::type_reference>();
        type->set_allocated_sized_int(sized.release());
        return type;
    }

    void sized_integer::_codegen_type(ir_generation_context &, std::shared_ptr<codegen::ir::user_type>) const
    {
        _codegen_t = codegen::ir::builtin_types().sized_integer(_size);
    }

    template<typename Instruction, typename Eval>
    auto sized_integer::_generate_function(std::u32string name,
        std::string desc,
        Eval eval,
        type * return_type)
    {
        auto lhs = make_runtime_value(this);
        auto rhs = make_runtime_value(this);

        auto lhs_arg = lhs.get();
        auto rhs_arg = rhs.get();

        auto fun = make_function(desc);
        fun->set_name(name);
        fun->set_return_type(return_type->get_expression());
        fun->set_parameters({ lhs_arg, rhs_arg });
        fun->set_eval(eval);
        fun->mark_builtin();
        fun->set_codegen([name = std::move(name), return_type, lhs = std::move(lhs), rhs = std::move(rhs)](
                             ir_generation_context & ctx) {
            auto lhs_ir = get_ir_variable(lhs->codegen_ir(ctx));
            auto rhs_ir = get_ir_variable(rhs->codegen_ir(ctx));

            auto retval = codegen::ir::make_variable(return_type->codegen_type(ctx));

            return codegen::ir::function{ name,
                {},
                { lhs_ir, rhs_ir },
                retval,
                { codegen::ir::instruction{ std::nullopt,
                      std::nullopt,
                      { boost::typeindex::type_id<Instruction>() },
                      { lhs_ir, rhs_ir },
                      retval },
                    codegen::ir::instruction{ std::nullopt,
                        std::nullopt,
                        { boost::typeindex::type_id<codegen::ir::return_instruction>() },
                        {},
                        retval } } };
        });
        return fun;
    }

#define EXPAND(...) __VA_ARGS__

#define ADD_OPERATION(NAME, BUILTIN_NAME, OPERATOR, RESULT_TYPE, CONSTANT_TYPE, ADDITIONAL_ARG)              \
    {                                                                                                        \
        auto eval = [=](auto &&, const std::vector<expression *> & args) {                                   \
            assert(args.size() == 2);                                                                        \
            assert(args[0]->get_type() == this);                                                             \
            assert(args[1]->get_type() == this);                                                             \
                                                                                                             \
            if (!args[0]->is_constant() || !args[1]->is_constant())                                          \
            {                                                                                                \
                return make_ready_future<expression *>(nullptr);                                             \
            }                                                                                                \
                                                                                                             \
            auto lhs = args[0]->as<sized_integer_constant>();                                                \
            auto rhs = args[1]->as<sized_integer_constant>();                                                \
            return make_ready_future<expression *>(std::make_unique<CONSTANT_TYPE##_constant>(               \
                EXPAND ADDITIONAL_ARG lhs->get_value() OPERATOR rhs->get_value())                            \
                                                       .release());                                          \
        };                                                                                                   \
        _##NAME = _generate_function<codegen::ir::integer_##NAME##_instruction>(BUILTIN_NAME,                \
            "<builtin sized_integer(" + std::to_string(_size) + ") " #NAME ">",                              \
            eval,                                                                                            \
            RESULT_TYPE);                                                                                    \
    }

    sized_integer::sized_integer(std::size_t size) : _size{ size }
    {
        auto u32size = utf32(std::to_string(size));

        ADD_OPERATION(addition,
            U"__builtin_sized_integer_" + u32size + U"_operator_plus",
            +,
            this,
            sized_integer,
            (this, ));
        ADD_OPERATION(subtraction,
            U"__builtin_sized_integer_" + u32size + U"_operator_minus",
            -,
            this,
            sized_integer,
            (this, ));
        ADD_OPERATION(
            multiplication, U"__builtin_sized_integer_" + u32size + U"_operator_star", *, this, sized_integer, (this, ));
        ADD_OPERATION(division,
            U"__builtin_sized_integer_" + u32size + U"_operator_slash",
            /,
            this,
            sized_integer,
            (this, ));
        ADD_OPERATION(equal_comparison,
            U"__builtin_sized_integer_" + u32size + U"_operator_equals",
            ==,
            builtin_types().boolean.get(),
            boolean,
            ());
        ADD_OPERATION(less_comparison,
            U"__builtin_sized_integer_" + u32size + U"_operator_less",
            <,
            builtin_types().boolean.get(),
            boolean,
            ());
        ADD_OPERATION(less_equal_comparison,
            U"__builtin_sized_" + u32size + U"_integer_operator_less_equal",
            <=,
            builtin_types().boolean.get(),
            boolean,
            ());

        _max_value = (boost::multiprecision::cpp_int(1) << _size) - 1;
        _min_value = -_max_value - 1;
    }
}
}
