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

#include "vapor/analyzer/symbol.h"
#include "vapor/codegen/ir/variable.h"

std::vector<reaver::variant<std::shared_ptr<reaver::vapor::codegen::_v1::ir::variable>, reaver::vapor::codegen::_v1::ir::function>> reaver::vapor::analyzer::_v1::symbol::codegen_ir(reaver::vapor::analyzer::_v1::ir_generation_context & ctx) const
{
    return fmap(_variable->codegen_ir(ctx), [](auto && v) {
        return fmap(std::forward<decltype(v)>(v), make_overload_set(
            [](codegen::ir::function f) { return std::move(f); },
            [](codegen::ir::value val) {
                return get<std::shared_ptr<codegen::ir::variable>>(fmap(std::move(val), make_overload_set(
                    [](std::shared_ptr<codegen::ir::variable> var) { return std::move(var); },
                    [](auto &&) { assert(0); return unit{}; }
                )));
            }));
        });
}

