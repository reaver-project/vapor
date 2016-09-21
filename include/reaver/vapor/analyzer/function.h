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

#pragma once

#include <memory>
#include <vector>

#include <reaver/function.h>
#include <reaver/optional.h>

#include "../range.h"
#include "../codegen/ir/function.h"

namespace reaver
{
    namespace vapor
    {
        namespace analyzer { inline namespace _v1
        {
            class type;

            class function
            {
            public:
                function(std::string explanation, std::shared_ptr<type> ret, std::vector<std::shared_ptr<type>> args, codegen::ir::function codegen, optional<range_type> range = none)
                    : _explanation{ std::move(explanation) }, _range{ std::move(range) },
                    _return_type{ std::move(ret) }, _argument_types{ std::move(args) }, _codegen{ std::move(codegen) }
                {
                }

                std::shared_ptr<type> return_type() const
                {
                    return _return_type;
                }

                std::vector<std::shared_ptr<type>> arguments() const
                {
                    return _argument_types;
                }

                std::string explain() const
                {
                    auto ret = _explanation;
                    fmap(_range, [&ret](auto && r) {
                        std::stringstream stream;
                        stream << " (at " << r << ")";
                        ret += stream.str();

                        return unit{};
                    });
                    return ret;
                }

                codegen::ir::function codegen_ir() const
                {
                    return _codegen;
                }

                codegen::ir::value call_operand_ir() const
                {
                    return { codegen::ir::label{ _codegen.name } };
                }

            private:
                std::string _explanation;
                optional<range_type> _range;

                std::shared_ptr<type> _return_type;
                std::vector<std::shared_ptr<type>> _argument_types;
                codegen::ir::function _codegen;
            };

            inline std::shared_ptr<function> make_function(std::string expl, std::shared_ptr<type> return_type, std::vector<std::shared_ptr<type>> arguments, codegen::ir::function codegen, optional<range_type> range = none)
            {
                return std::make_shared<function>(std::move(expl), std::move(return_type), std::move(arguments), std::move(codegen), std::move(range));
            }
        }}
    }
}

