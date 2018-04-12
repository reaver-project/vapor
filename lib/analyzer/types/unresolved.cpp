/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/types/unresolved.h"

#include <boost/functional/hash.hpp>

#include "vapor/analyzer/expressions/expression_ref.h"
#include "vapor/analyzer/expressions/unresolved_type.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/analyzer/types/sized_integer.h"

#include <google/protobuf/util/message_differencer.h>
#include "expressions/type.pb.h"
#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    bool user_defined_reference_compare::operator()(const proto::user_defined_reference * lhs, const proto::user_defined_reference * rhs) const
    {
        return google::protobuf::util::MessageDifferencer::Equals(*lhs, *rhs);
    }

    std::size_t user_defined_reference_hash::operator()(const proto::user_defined_reference * udr) const
    {
        std::size_t seed = 0;

        for (auto && module : udr->module())
        {
            boost::hash_combine(seed, module);
        }

        for (auto && scope : udr->scope())
        {
            boost::hash_combine(seed, scope);
        }

        boost::hash_combine(seed, udr->name());

        return seed;
    }

    std::unique_ptr<expression> unresolved_type::get_expression() const
    {
        return std::make_unique<unresolved_type_expression>(shared_from_this());
    }

    std::unique_ptr<expression> get_imported_type(precontext & ctx, const proto::type & type)
    {
        switch (type.details_case())
        {
            case proto::type::DetailsCase::kReference:
                switch (type.reference().details_case())
                {
                    case proto::type_reference::DetailsCase::kBuiltin:
                    case proto::type_reference::DetailsCase::kSizedInt:
                        return make_expression_ref(std::get<0>(get_imported_type_ref(ctx, type.reference()))->get_expression());

                    case proto::type_reference::DetailsCase::kUserDefined:
                        return std::get<1>(get_imported_type_ref(ctx, type.reference()))->get_expression();

                    default:
                        assert(0);
                }

            case proto::type::DetailsCase::kStruct:
                assert(0);

            case proto::type::DetailsCase::kOverloadSet:
                assert(0);

            default:
                assert(0);
        }
    }

    imported_type get_imported_type_ref(precontext & ctx, const proto::type_reference & type)
    {
        switch (type.details_case())
        {
            case proto::type_reference::DetailsCase::kBuiltin:
                switch (type.builtin())
                {
                    case proto::type_:
                        return builtin_types().type.get();
                    case proto::integer:
                        return builtin_types().integer.get();
                    case proto::boolean:
                        return builtin_types().boolean.get();

                    default:
                        assert(0);
                }

            case proto::type_reference::DetailsCase::kSizedInt:
            {
                auto size = type.sized_int().size();
                auto & type = ctx.proper.sized_integers[size];
                if (!type)
                {
                    type = make_sized_integer_type(size);
                }
                return type.get();
            }

            case proto::type_reference::kUserDefined:
            {
                auto ud = ctx.user_defined_types[&type.user_defined()];
                if (!ud)
                {
                    ud = std::make_shared<unresolved_type>(&type.user_defined());
                }

                return ud;
            }

            default:
                assert(0);
        }
    }
}
}
