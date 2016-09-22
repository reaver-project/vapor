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

#include <numeric>

#include <reaver/prelude/monad.h>

#include "vapor/codegen/result.h"
#include "vapor/codegen/ir/module.h"
#include "vapor/codegen/generator.h"

reaver::vapor::codegen::_v1::result::result(std::vector<reaver::vapor::codegen::_v1::ir::module> ir, std::shared_ptr<code_generator> gen)
{
    auto ctx = codegen_context{gen};

    auto strings = mbind(ir, [&](auto && module) {
        return fmap(module.symbols, [&](auto && symbol) {
            return get<std::u32string>(fmap(symbol, make_overload_set(
                [&](std::shared_ptr<ir::variable> & var) {
                    return gen->generate(*var, ctx);
                },
                [&](ir::function & fn) {
                    return gen->generate(fn, ctx);
                }
            )));
        });
    });

    _generated_code = std::accumulate(strings.begin(), strings.end(), std::u32string{});
}

