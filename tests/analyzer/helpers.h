/**
 * Vapor Compiler Licence
 *
 * Copyright © 2017 Michał "Griwes" Dominiak
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

#include <reaver/mayfly.h>

#include "vapor/analyzer.h"
#include "vapor/analyzer/symbol.h"
#include "vapor/lexer.h"
#include "vapor/parser.h"

namespace reaver::vapor::analyzer
{
inline namespace _v1
{
    template<typename F>
    auto parse(std::u32string program, F && parser)
    {
        parser::context ctx;
        ctx.begin = lexer::iterator{ program.begin(), program.end() };

        return parser(ctx);
    }

    struct trivial_executor : public reaver::executor
    {
        virtual void push(reaver::function<void()> f) override
        {
            if (in_function)
            {
                queue.push_back(std::move(f));
                return;
            }

            in_function = true;
            f();

            while (queue.size())
            {
                auto fs = move(queue);
                queue.clear();

                for (auto && f : fs)
                {
                    f();
                }
            }

            in_function = false;
        }

        std::vector<reaver::function<void()>> queue;
        bool in_function = false;
    };

    class unexpected_call : public reaver::exception
    {
    public:
        unexpected_call(const char * desc) : exception{ reaver::logger::fatal }
        {
            *this << desc;
        }
    };

    class test_variable : public variable
    {
    public:
        test_variable(type * t) : _type{ t }
        {
        }

        virtual type * get_type() const override
        {
            return _type;
        }

        void set_clone_result(std::unique_ptr<variable> var)
        {
            _clone_var = std::move(var);
        }

    private:
        virtual std::unique_ptr<variable> _clone_with_replacement(replacements &) const override
        {
            if (!_clone_var)
            {
                throw unexpected_call{ __PRETTY_FUNCTION__ };
            }

            return std::move(const_cast<test_variable *>(this)->_clone_var);
        }

        virtual variable_ir _codegen_ir(ir_generation_context &) const override
        {
            throw unexpected_call{ __PRETTY_FUNCTION__ };
        }

        type * _type = nullptr;
        std::unique_ptr<variable> _clone_var;
    };

    class test_expression : public expression
    {
    public:
        test_expression()
        {
        }

        test_expression(type * t) : _type{ t }
        {
            _set_variable(std::make_unique<test_variable>(t));
        }

        virtual void print(std::ostream &, std::size_t) const override
        {
            throw unexpected_call{ __PRETTY_FUNCTION__ };
        }

        void set_variable(std::unique_ptr<variable> var)
        {
            _set_variable(std::move(var));
        }

        void set_analysis_variable(std::unique_ptr<variable> var)
        {
            _analysis_variable = std::move(var);
        }

        void set_clone_result(std::unique_ptr<expression> expr)
        {
            _clone_expr = std::move(expr);
        }

        void set_simplified_expression(std::unique_ptr<expression> expr)
        {
            _simplified_expr = std::move(expr);
        }

    private:
        virtual reaver::future<> _analyze(analysis_context &) override
        {
            if (!_analysis_variable)
            {
                throw unexpected_call{ __PRETTY_FUNCTION__ };
            }

            _set_variable(std::move(_analysis_variable));

            return reaver::make_ready_future();
        }

        virtual std::unique_ptr<expression> _clone_expr_with_replacement(replacements &) const override
        {
            if (!_clone_expr)
            {
                throw unexpected_call{ __PRETTY_FUNCTION__ };
            }

            return std::move(const_cast<test_expression *>(this)->_clone_expr);
        }

        virtual reaver::future<expression *> _simplify_expr(simplification_context &) override
        {
            if (!_simplified_expr)
            {
                throw unexpected_call{ __PRETTY_FUNCTION__ };
            }

            return make_ready_future(_simplified_expr.release());
        }

        virtual statement_ir _codegen_ir(ir_generation_context &) const override
        {
            throw unexpected_call{ __PRETTY_FUNCTION__ };
        }

        type * _type = nullptr;
        std::unique_ptr<expression> _clone_expr;
        std::unique_ptr<expression> _simplified_expr;
        std::unique_ptr<variable> _analysis_variable;
    };

    class test_type : public type
    {
    public:
        test_type() = default;

        virtual std::string explain() const override
        {
            return "test type";
        }

    private:
        virtual void _codegen_type(ir_generation_context &) const override
        {
            throw unexpected_call(__PRETTY_FUNCTION__);
        }
    };
}
}
