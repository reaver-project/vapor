/**
 * Vapor Compiler Licence
 *
 * Copyright © 2016-2019 Michał "Griwes" Dominiak
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

#include "vapor/analyzer/types/type.h"

#include <reaver/id.h>

#include "vapor/analyzer/expressions/call.h"
#include "vapor/analyzer/expressions/runtime_value.h"
#include "vapor/analyzer/expressions/type.h"
#include "vapor/analyzer/precontext.h"
#include "vapor/analyzer/semantic/function.h"
#include "vapor/analyzer/semantic/overloads.h"
#include "vapor/analyzer/semantic/symbol.h"
#include "vapor/analyzer/types/pack.h"
#include "vapor/analyzer/types/sized_integer.h"
#include "vapor/analyzer/types/unconstrained.h"

#include "expressions/type.pb.h"
#include "type_reference.pb.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    type::~type() = default;

    void type::_init_expr()
    {
        _self_expression = make_type_expression(this);
    }

    void type::_init_pack_type()
    {
        _pack_type = make_pack_type(this);
    }

    expression * type::get_expression() const
    {
        return _self_expression->_get_replacement();
    }

    class type_type : public type
    {
    public:
        type_type() : type{ dont_init_expr }
        {
        }

        virtual std::string explain() const override
        {
            return "type";
        }

        virtual future<std::vector<function *>> get_candidates(lexer::token_type token) const override
        {
            if (token != lexer::token_type::curly_bracket_open)
            {
                assert(0);
                return make_ready_future(std::vector<function *>{});
            }

            [&] {
                std::lock_guard<std::mutex> lock{ _generic_ctor_lock };

                if (_generic_ctor)
                {
                    return;
                }

                _generic_ctor_first_arg = make_runtime_value(builtin_types().type.get());
                _generic_ctor_pack_arg = make_runtime_value(builtin_types().unconstrained->get_pack_type());

                _generic_ctor = make_function("generic constructor");
                _generic_ctor->set_return_type(_generic_ctor_first_arg.get());
                _generic_ctor->set_parameters({ _generic_ctor_first_arg.get(), _generic_ctor_pack_arg.get() });

                _generic_ctor->make_member();

                _generic_ctor->add_analysis_hook([](auto && ctx, auto && call_expr, auto && args) {
                    assert(args.size() != 0);
                    assert(args.front()->get_type() == builtin_types().type.get());
                    assert(args.front()->is_constant());

                    auto type_expr = args.front()->template as<type_expression>();
                    auto actual_type = type_expr->get_value();
                    args.erase(args.begin());
                    auto actual_ctor = actual_type->get_constructor(fmap(args, [](auto && arg) -> const expression * { return arg; }));

                    return actual_ctor.then([&ctx, args, call_expr](auto && ctor) { return select_overload(ctx, call_expr->get_range(), args, { ctor }); })
                        .then([call_expr](auto && expr) { call_expr->replace_with(std::move(expr)); });
                });

                _generic_ctor->set_eval(
                    [](auto &&, auto &&) -> future<expression *> { assert(!"a generic constructor call survived analysis; this is a compiler bug"); });
            }();

            return make_ready_future(std::vector<function *>{ _generic_ctor.get() });
        }

        virtual void print(std::ostream & os, print_context ctx) const override
        {
            os << styles::def << ctx << styles::type << "type" << styles::def << " @ " << styles::address << this << styles::def << ": builtin type\n";
        }

        virtual std::unique_ptr<proto::type> generate_interface() const override
        {
            auto ret = std::make_unique<proto::type>();
            ret->set_allocated_reference(generate_interface_reference().release());
            return ret;
        }

        virtual std::unique_ptr<proto::type_reference> generate_interface_reference() const override
        {
            auto ret = std::make_unique<proto::type_reference>();
            ret->set_builtin(proto::type_);
            return ret;
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override
        {
            assert(0);
        }

        virtual std::u32string _codegen_name(ir_generation_context & ctx) const override
        {
            return U"type";
        }

        mutable std::mutex _generic_ctor_lock;
        mutable std::shared_ptr<function> _generic_ctor;
        mutable std::unique_ptr<expression> _generic_ctor_first_arg;
        mutable std::unique_ptr<expression> _generic_ctor_pack_arg;
    };

    std::unique_ptr<type> make_type_type()
    {
        return std::make_unique<type_type>();
    }

    std::unique_ptr<proto::type> user_defined_type::generate_interface() const
    {
        auto ret = std::make_unique<proto::type>();

        auto message = _user_defined_interface();

        auto dynamic_switch = [&](auto &&... pairs) {
            ((dynamic_cast<typename decltype(pairs.first)::type *>(message.get())
                 && pairs.second(static_cast<typename decltype(pairs.first)::type *>(message.release())))
                    || ... || [&]() -> bool {
                auto m = message.get();
                throw exception{ logger::crash } << "unhandled serialized type type: `" << typeid(*m).name() << "`";
            }());
        };

#define HANDLE_TYPE(type, field_name)                                                                                                                          \
    std::make_pair(id<proto::type>(), [&](auto ptr) {                                                                                                          \
        ret->set_allocated_##field_name(ptr);                                                                                                                  \
        return true;                                                                                                                                           \
    })

        dynamic_switch(HANDLE_TYPE(overload_set_type, overload_set), HANDLE_TYPE(struct_type, struct_));

#undef HANDLE_TYPE

        return ret;
    }

    std::unique_ptr<proto::type_reference> user_defined_type::generate_interface_reference() const
    {
        auto user_defined = std::make_unique<proto::user_defined_reference>();

        for (auto scope : get_scope()->codegen_ir())
        {
            switch (scope.type)
            {
                case codegen::ir::scope_type::module:
                    *user_defined->add_module() = utf8(scope.name);
                    break;

                case codegen::ir::scope_type::type:
                    *user_defined->add_scope() = utf8(scope.name);
                    break;

                default:
                    assert(0);
            }
        }

        user_defined->set_name(utf8(get_name()));

        auto ret = std::make_unique<proto::type_reference>();
        ret->set_allocated_user_defined(user_defined.release());

        return ret;
    }
}
}
