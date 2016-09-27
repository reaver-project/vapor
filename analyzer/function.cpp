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

#include "vapor/analyzer/function.h"
#include "vapor/analyzer/block.h"

reaver::future<> reaver::vapor::analyzer::_v1::function::simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx)
{
    auto body = _body.lock();
    if (body)
    {
        return body->simplify(ctx)
            .then([&](auto && simplified) {
                _body = std::dynamic_pointer_cast<block>(std::move(simplified));
            });
    }

    return make_ready_future();
}

reaver::future<std::shared_ptr<reaver::vapor::analyzer::_v1::expression>> reaver::vapor::analyzer::_v1::function::simplify(reaver::vapor::analyzer::_v1::optimization_context & ctx, std::vector<std::shared_ptr<variable>> args)
{
    if (auto body = _body.lock())
    {
        // need to do something smart with arguments in this case
        return make_ready_future(std::shared_ptr<expression>());
    }

    if (_compile_time_eval)
    {
        return make_ready_future((*_compile_time_eval)(ctx, args));
    }

    assert(0);
}

