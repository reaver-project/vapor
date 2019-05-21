/**
 * Vapor Compiler Licence
 *
 * Copyright © 2019 Michał "Griwes" Dominiak
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

#include "vapor/print_helpers.h"
#include "vapor/analyzer/precontext.h"

#include "range.pb.h"

namespace reaver::vapor
{
inline namespace _v1
{
    ast_node imported_ast_node(analyzer::precontext & ctx, const proto::range & r)
    {
        auto import_position = [&ctx](const proto::position p) {
            position ret;
            ret.offset = p.offset();
            ret.line = p.line();
            ret.column = p.column();
            ret.file_path = ctx.module_path_stack.back().source;
            return ret;
        };

        return { nullptr, { import_position(r.start()), import_position(r.end()) } };
    }
}
}
