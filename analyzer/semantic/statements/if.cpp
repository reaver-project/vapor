/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2017 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/statements/if.h"
#include "vapor/analyzer/helpers.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/parser/lambda_expression.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    future<> if_statement::_analyze(analysis_context & ctx)
    {
        auto fut = _condition->analyze(ctx);

        auto analyze_block = [&](auto && block) {
            fut = fut.then([&]() {
                auto tmp_ctx = std::make_unique<analysis_context>(ctx);
                auto fut = block->analyze(*tmp_ctx);
                return fut.then([ctx = std::move(tmp_ctx)]{});
            });
            return unit{};
        };

        analyze_block(_then_block);
        fmap(_else_block, analyze_block);

        return fut;
    }
}
}
