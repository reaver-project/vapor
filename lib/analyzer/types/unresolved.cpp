/**
 * Vapor Compiler Licence
 *
 * Copyright © 2018-2019 Michał "Griwes" Dominiak
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
#include "vapor/analyzer/expressions/struct_literal.h"
#include "vapor/analyzer/expressions/unresolved_type.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/types/archetype.h"
#include "vapor/analyzer/types/overload_set.h"
#include "vapor/analyzer/types/sized_integer.h"
#include "vapor/analyzer/types/struct.h"
#include "vapor/analyzer/types/typeclass.h"

#include <google/protobuf/util/message_differencer.h>
#include "expressions/type.pb.h"
#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    bool user_defined_reference_compare::operator()(const proto::user_defined_reference * lhs,
        const proto::user_defined_reference * rhs) const
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

    unresolved_type::unresolved_type(const proto::user_defined_reference * reference, scope * lex_scope)
        : _reference{ _unresolved_reference{ lex_scope,
            std::make_unique<proto::user_defined_reference>(*reference) } }
    {
    }

    unresolved_type::unresolved_type(typeclass_type_tag, std::vector<imported_type> params)
        : _reference{ _unresolved_typeclass_type{ std::move(params) } }
    {
    }

    unresolved_type::unresolved_type(archetype_type_tag, scope * lex_scope, std::u32string name)
        : _reference{ _unresolved_archetype_type{ lex_scope, std::move(name) } }
    {
    }

    future<> unresolved_type::resolve(analysis_context & ctx)
    {
        if (!_analysis_future)
        {
            _analysis_future = std::get<0>(fmap(_reference,
                make_overload_set([&](std::monostate) -> future<> { assert(0); },

                    [&](_unresolved_reference & ref) -> future<> {
                        auto go_one_level = [&ctx](scope * lex_scope, const std::string & name) {
                            auto symb = lex_scope->try_get(utf32(name));
                            assert(symb
                                && "currently, using unexported types in exported signatures is not allowed; "
                                   "this will change in the future");
                            auto expr = symb.value()->get_expression();
                            return expr->analyze(ctx).then([expr] { return expr->get_type()->get_scope(); });
                        };

                        auto kinds = { &ref.udr->module(), &ref.udr->scope() };
                        return std::accumulate(kinds.begin(),
                            kinds.end(),
                            make_ready_future(ref.lex_scope),
                            [&, go_one_level](auto future, auto && kind) {
                                return std::accumulate(kind->begin(),
                                    kind->end(),
                                    future,
                                    [go_one_level](auto future, const auto & name) {
                                        return future.then([&name, go_one_level](auto scope) {
                                            return go_one_level(scope, name);
                                        });
                                    });
                            })
                            .then([&ctx, &ref, this](scope * lex_scope) {
                                auto symb = lex_scope->try_get(utf32(ref.udr->name()));
                                assert(symb
                                    && "currently, using unexported types in exported signatures is "
                                       "not allowed; this will change in the future");

                                auto expr = symb.value()->get_expression();
                                return expr->analyze(ctx).then([expr, this] {
                                    auto type_expr = expr->as<type_expression>();
                                    assert(type_expr);
                                    _resolved = type_expr->get_value();
                                });
                            });
                    },

                    [&](_unresolved_typeclass_type & tc) -> future<> {
                        return when_all(fmap(tc.parameter_types, [&](auto && imported) {
                            return std::get<0>(fmap(imported,
                                make_overload_set(
                                    [&](type * resolved) { return make_ready_future(resolved); },
                                    [&](std::shared_ptr<unresolved_type> unresolved) {
                                        return unresolved->resolve(ctx).then(
                                            [unresolved] { return unresolved->get_resolved(); });
                                    })));
                        })).then([&](auto && types) { _resolved = ctx.get_typeclass_type(types); });
                    },

                    [&](_unresolved_archetype_type & arch) -> future<> {
                        auto && symb = arch.lex_scope->get(arch.name);
                        auto && type_expr = symb->get_expression()->as<type_expression>();
                        assert(type_expr);
                        assert(dynamic_cast<archetype *>(type_expr->get_value()));
                        _resolved = type_expr->get_value();

                        return make_ready_future();
                    })));
        }

        return _analysis_future.value();
    }

    std::unique_ptr<expression> unresolved_type::get_expression()
    {
        return std::make_unique<unresolved_type_expression>(shared_from_this());
    }

    std::unique_ptr<expression> get_imported_type(precontext & ctx, const proto::type & type)
    {
        switch (type.details_case())
        {
            case proto::type::DetailsCase::kReference:
                return get_imported_type_ref_expr(ctx, type.reference());

            case proto::type::DetailsCase::kStruct:
            {
                auto ret =
                    std::make_unique<struct_literal>(ast_node{}, import_struct_type(ctx, type.struct_()));
                ret->set_name(utf32(ctx.current_symbol));
                return ret;
            }

            case proto::type::DetailsCase::kOverloadSet:
                assert(!"overload set types should be associated!");

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
                return ctx.proper.get_sized_integer_type(type.sized_int().size());

            case proto::type_reference::kTypeclass:
            {
                std::vector<proto::type_reference> params{ type.typeclass().parameters().begin(),
                    type.typeclass().parameters().end() };
                return std::make_shared<unresolved_type>(unresolved_typeclass_type,
                    fmap(params, [&](auto && type) { return get_imported_type_ref(ctx, type); }));
            }

            case proto::type_reference::kUserDefined:
            {
                auto & ud = ctx.user_defined_types[&type.user_defined()];
                if (!ud)
                {
                    ud = std::make_shared<unresolved_type>(&type.user_defined(), ctx.global_scope);
                }

                return ud;
            }

            case proto::type_reference::kTypeclassInstanceType:
                assert(0);

            case proto::type_reference::kArchetype:
                return std::make_shared<unresolved_type>(
                    unresolved_archetype_type, ctx.current_lex_scope, utf32(type.archetype().name()));

            default:
                assert(0);
        }
    }

    std::unique_ptr<expression> get_imported_type_ref_expr(precontext & ctx,
        const proto::type_reference & reference)
    {
        switch (reference.details_case())
        {
            case proto::type_reference::DetailsCase::kBuiltin:
            case proto::type_reference::DetailsCase::kSizedInt:
                return make_expression_ref(
                    std::get<0>(get_imported_type_ref(ctx, reference))->get_expression(), std::nullopt);

            case proto::type_reference::DetailsCase::kUserDefined:
            case proto::type_reference::DetailsCase::kTypeclass:
            case proto::type_reference::kArchetype:
                return std::get<1>(get_imported_type_ref(ctx, reference))->get_expression();

            case proto::type_reference::DetailsCase::kTypeclassInstanceType:
                assert(0);

            default:
                assert(0);
        }
    }
}
}
