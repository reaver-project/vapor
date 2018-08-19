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

#include "vapor/analyzer/expressions/typeclass.h"
#include "vapor/analyzer/statements/function.h"
#include "vapor/analyzer/symbol.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> typeclass_literal::_analyze(analysis_context & ctx)
    {
        return _parameters_set->then([&] {
            return when_all(fmap(_instance_template->get_member_function_decls(), [&](auto && decl) {
                decl->set_template_parameters(_instance_template->get_template_parameters());
                return decl->analyze(ctx);
            }));
        });
    }

    std::unique_ptr<expression> typeclass_literal::_do_instantiate(analysis_context & ctx, std::vector<expression *> arguments) const
    {
        return std::make_unique<typeclass_literal_instance>(_instance_template.get(), std::move(arguments));
    }
}
}
