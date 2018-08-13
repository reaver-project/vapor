/**
 * Vapor Compiler Licence
 *
 * Copyright © 2014-2018 Michał "Griwes" Dominiak
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

#include <google/protobuf/any.pb.h>

#include <reaver/prelude/monad.h>

#include "../helpers.h"
#include "../statements/statement.h"
#include "../types/type.h"
#include "context.h"

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct expression;
    struct expression_list;
}
}

namespace reaver::vapor::proto
{
struct entity;
}

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    class scope;

    constexpr struct imported_tag_t
    {
    } imported_tag;

    class expression : public statement
    {
    public:
        expression() = default;
        virtual ~expression() = default;

        expression(type * t) : _type{ t }
        {
        }

        type * get_type() const
        {
            if (!_type)
            {
                assert(!"tried to get an unset type");
            }

            return _type;
        }

        type * try_get_type() const
        {
            return _type;
        }

        friend class replacements;

    private:
        std::unique_ptr<expression> clone_expr_with_replacement(replacements & repl) const
        {
            return _clone_expr_with_replacement(repl);
        }

    public:
        future<expression *> simplify_expr(recursive_context ctx)
        {
            return ctx.proper.get_future_or_init(this, [&]() { return make_ready_future().then([this, ctx]() { return _simplify_expr(ctx); }); });
        }

        void set_context(expression_context ctx)
        {
            _expr_ctx = std::move(ctx);
        }

        const expression_context & get_context() const
        {
            return _expr_ctx;
        }

        expression * get_default_value() const
        {
            return _default_value;
        }

        void set_default_value(expression * expr)
        {
            assert(!_default_value);
            _default_value = expr;
        }

        virtual void set_template_parameters(std::vector<parameter *>) override
        {
            assert(!"this expression doesn't support being a template");
        }

        virtual bool is_constant() const
        {
            auto repl = _get_replacement();
            assert(repl);
            return repl != this && repl->is_constant();
        }

        bool is_equal(const expression * rhs) const
        {
            if (this == rhs && _is_pure())
            {
                return true;
            }

            if (_is_equal(rhs) || rhs->_is_equal(this))
            {
                return true;
            }

            auto this_replacement = _get_replacement();
            auto rhs_replacement = rhs->_get_replacement();
            if (this != this_replacement || rhs != rhs_replacement)
            {
                return this_replacement->_is_equal(rhs_replacement) || rhs_replacement->_is_equal(this_replacement);
            }

            return false;
        }

        bool is_different_constant(const expression * rhs)
        {
            bool is_c = is_constant();

            if (is_c ^ rhs->is_constant())
            {
                return true;
            }

            return is_c && !is_equal(rhs);
        }

        virtual std::unique_ptr<expression> convert_to(type * target) const
        {
            if (get_type() == target)
            {
                assert(0); // I don't know whether I want to support this
            }

            auto repl = _get_replacement();
            if (repl == this)
            {
                return nullptr;
            }

            return repl->convert_to(target);
        }

        virtual bool is_member() const
        {
            return false;
        }

        virtual bool is_member_assignment() const
        {
            auto repl = _get_replacement();
            return repl != this && repl->is_member_assignment();
        }

        virtual bool is_member_access() const
        {
            return false;
        }

        virtual expression * get_member(const std::u32string & name) const
        {
            auto repl = _get_replacement();
            if (repl == this)
            {
                return nullptr;
            }

            return repl->get_member(name);
        }

        template<typename T>
        T * as()
        {
            return dynamic_cast<T *>(_get_replacement());
        }

        template<typename T>
        const T * as() const
        {
            return dynamic_cast<const T *>(_get_replacement());
        }

        virtual std::size_t hash_value() const
        {
            return 0;
        }

        // this ought to be protected
        // but then derived classes wouldn't be able to recurse
        // so let's just mark it as "protected interface"
        // these aren't mutators, so nothing bad should ever happen
        // but I know these are famous last words
        virtual expression * _get_replacement()
        {
            return this;
        }

        virtual const expression * _get_replacement() const
        {
            return this;
        }

        void generate_interface(proto::entity &) const;

        std::unique_ptr<google::protobuf::Message> _do_generate_interface() const
        {
            return _generate_interface();
        }

        virtual void set_name([[maybe_unused]] std::u32string name)
        {
        }

        virtual void mark_exported()
        {
        }

    protected:
        void _set_type(type * t)
        {
            assert(!_type);
            assert(t);
            _type = t;
        }

        virtual future<> _analyze(analysis_context &) override
        {
            assert(_type);
            return make_ready_future();
        }

        std::unique_ptr<expression> _instantiate(analysis_context & ctx, std::vector<expression *> arguments) const;

        virtual std::unique_ptr<statement> _clone_with_replacement(replacements & repl) const final
        {
            return _clone_expr_with_replacement(repl);
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements & repl) const = 0;

        virtual future<statement *> _simplify(recursive_context ctx) override final
        {
            return simplify_expr(ctx).then([&](auto && simplified) -> statement * { return simplified; });
        }

        virtual future<expression *> _simplify_expr(recursive_context)
        {
            return make_ready_future(this);
        }

        virtual bool _is_equal([[maybe_unused]] const expression * expr) const
        {
            return false;
        }

        bool _is_pure() const
        {
            return true;
        }

        virtual std::unique_ptr<google::protobuf::Message> _generate_interface() const
        {
            auto replacement = _get_replacement();

            if (this == replacement)
            {
                assert(0);
            }

            return replacement->_generate_interface();
        }

    private:
        virtual std::unique_ptr<expression> _do_instantiate([[maybe_unused]] analysis_context & ctx, [[maybe_unused]] std::vector<expression *> arguments) const
        {
            assert(!"tried to instantiate an expression that's not instantiatable");
        }

        type * _type = nullptr;
        expression * _default_value = nullptr;

        expression_context _expr_ctx;
    };

    inline std::size_t hash_value(const expression & expr)
    {
        return expr.hash_value();
    }
}
}

namespace reaver::vapor::parser
{
inline namespace _v1
{
    struct expression;
}
}
namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    struct precontext;
    std::unique_ptr<expression> preanalyze_expression(precontext & ctx, const parser::expression & expr, scope * lex_scope);
}
}
