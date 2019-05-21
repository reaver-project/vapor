/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017, 2019 Michał "Griwes" Dominiak
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

#include <boost/algorithm/string/join.hpp>

#include "vapor/codegen/ir/instruction.h"
#include "vapor/codegen/ir/type.h"
#include "vapor/codegen/llvm_ir.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    template<>
    std::u32string llvm_ir_generator::generate<ir::aggregate_init_instruction>(const ir::instruction & inst,
        codegen_context & ctx)
    {
        std::u32string ret;

        std::u32string variable = variable_of(inst.result, ctx);
        variable.pop_back(); // get rid of the closing "

        std::size_t step_count = 0;
        std::u32string base_variable = U"undef";

        for (auto && argument : inst.operands)
        {
            std::u32string next_variable = [&] {
                if (step_count + 1 == inst.operands.size())
                {
                    return variable + U"\"";
                }

                return variable + U"." + utf32(std::to_string(step_count)) + U"\"";
            }();

            ret += next_variable + U" = insertvalue " + type_of(inst.result, ctx) + U" " + base_variable
                + U", " + type_of(argument, ctx) + U" " + value_of(argument, ctx) + U", "
                + utf32(std::to_string(step_count)) + U"\n";

            base_variable = std::move(next_variable);
            ++step_count;
        }

        return ret;

        std::u32string arguments;
        std::for_each(inst.operands.begin(), inst.operands.end(), [&](auto && operand) {
            arguments += value_of(operand, ctx);
            arguments += U", ";
        });
        if (inst.operands.size() > 0)
        {
            arguments.pop_back();
            arguments.pop_back();
        }

        return variable_of(inst.result, ctx) + U".emplace(" + arguments + U");\n";
    }

    template<>
    std::u32string llvm_ir_generator::generate<ir::member_access_instruction>(const ir::instruction & inst,
        codegen_context & ctx)
    {
        assert(inst.operands.size() == 2);

        auto index = std::get<std::size_t>(fmap(inst.operands[1],
            make_overload_set(
                [&](const ir::label & label) {
                    // TODO: the operand generated for this instruction should probably be ir::member_variable
                    // directly this also means that member_variable needs *index* in addition to offset
                    auto && member_name = label.name;
                    auto user_type = dynamic_cast<ir::user_type *>(
                        std::get<std::shared_ptr<ir::variable>>(inst.operands[0])->type.get());
                    assert(user_type);
                    auto & members = user_type->members;

                    std::size_t index = 0;
                    auto member_ir = std::find_if(members.begin(), members.end(), [&](auto && v) {
                        return std::get<bool>(fmap(v,
                            make_overload_set(
                                [&](const ir::member_variable & member) {
                                    ++index;
                                    return member.name == member_name;
                                },
                                [&](const ir::function &) { return false; })));
                    });

                    assert(member_ir != members.end());

                    return index - 1;
                },
                [&](const ir::integer_value & index) { return index.value.convert_to<std::size_t>(); },
                [](auto &&) -> std::size_t { assert(0); })));

        return variable_of(inst.result, ctx) + U" = extractvalue " + type_of(inst.operands[0], ctx) + U" "
            + value_of(inst.operands[0], ctx) + U", " + utf32(std::to_string(index)) + U"\n";
    }
}
}
