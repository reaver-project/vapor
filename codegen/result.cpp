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

#include "vapor/codegen/generator.h"
#include "vapor/codegen/ir/module.h"
#include "vapor/codegen/result.h"

namespace reaver::vapor::codegen
{
inline namespace _v1
{
    result::result(std::vector<ir::module> ir, std::shared_ptr<code_generator> gen)
    {
        auto ctx = codegen_context{ gen };

        _generated_code = UR"code(#include <type_traits>
#include <utility>
#include <reaver/manual.h>
    )code";

        fmap(ir, [&](auto && module) {
            fmap(module.name, [&](auto && token) {
                _generated_code += U"namespace " + token + U"\n{\n";
                return unit{};
            });

            fmap(module.symbols, [&](auto && symbol) {
                return fmap(symbol,
                    make_overload_set(
                        [&](std::shared_ptr<ir::variable> & var) {
                            _generated_code += gen->generate_declaration(*var, ctx);
                            return unit{};
                        },
                        [&](ir::function & fn) {
                            _generated_code += gen->generate_declaration(fn, ctx);
                            return unit{};
                        }));
            });

            fmap(module.name, [&](auto &&) {
                _generated_code += U"}\n";
                return unit{};
            });

            return unit{};
        });

        auto declarations = ctx.put_into_global_before + _generated_code + ctx.put_into_global;
        _generated_code = U"";
        ctx.put_into_global_before = U"";
        ctx.put_into_global = U"";

        fmap(ir, [&](auto && module) {
            fmap(module.symbols, [&](auto && symbol) {
                return fmap(symbol,
                    make_overload_set(
                        [&](std::shared_ptr<ir::variable> & var) {
                            _generated_code += gen->generate_definition(*var, ctx);
                            return unit{};
                        },
                        [&](ir::function & fn) {
                            _generated_code += gen->generate_definition(fn, ctx);
                            return unit{};
                        }));
            });

            return unit{};
        });

        _generated_code = declarations + ctx.put_into_global_before + _generated_code + ctx.put_into_global;
    }
}
}
