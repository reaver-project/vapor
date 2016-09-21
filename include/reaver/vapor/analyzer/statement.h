/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014, 2016 Michał "Griwes" Dominiak
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

#pragma once

#include <reaver/future.h>

#include "../codegen/ir/variable.h"
#include "../codegen/ir/instruction.h"
#include "ir_context.h"

namespace reaver
{
    namespace vapor
    {
        namespace parser { inline namespace _v1
        {
            struct statement;
        }}

        namespace analyzer { inline namespace _v1
        {
            class return_statement;
            class scope;

            using statement_ir = std::vector<codegen::ir::instruction>;

            class statement
            {
            public:
                statement() = default;
                virtual ~statement() = default;

                future<> analyze()
                {
                    if (!_is_future_assigned)
                    {
                        std::lock_guard<std::mutex> lock{ _future_lock };
                        if (!_is_future_assigned)
                        {
                            _analysis_future = _analyze();
                        }
                    }

                    return *_analysis_future;
                }

                virtual std::vector<std::shared_ptr<const return_statement>> get_returns() const
                {
                    return {};
                }

                virtual void print(std::ostream &, std::size_t indent) const = 0;

                statement_ir codegen_ir(ir_generation_context & ctx) const
                {
                    if (!_ir)
                    {
                        _ir = _codegen_ir(ctx);
                    }

                    return *_ir;
                }

            private:
                virtual future<> _analyze() = 0;
                virtual statement_ir _codegen_ir(ir_generation_context &) const = 0;

                std::mutex _future_lock;
                std::atomic<bool> _is_future_assigned{ false };
                optional<future<>> _analysis_future;
                mutable optional<statement_ir> _ir;
            };

            std::shared_ptr<statement> preanalyze_statement(const parser::statement & parse, std::shared_ptr<scope> & lex_scope);
        }}
    }
}

