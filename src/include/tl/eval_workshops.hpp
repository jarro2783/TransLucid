/* Workshops generated from AST::Expr.
   Copyright (C) 2009--2012 Jarryd Beck

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef TL_EVAL_WORKSHOP_HPP_INCLUDED
#define TL_EVAL_WORKSHOP_HPP_INCLUDED

/**
 * @file compiled_functors.hpp
 * The header file for the compiled hyperdatons which run to evaluate
 * expressions.
 */

//#include <tl/ast.hpp>
#include <tl/builtin_types.hpp>
#include <tl/system.hpp>
#include <tl/types/special.hpp>
#include <tl/workshop.hpp>

#include <list>
#include <set>

//Some of these functors are constructed with the system, some are not.
//The ones that are make a demand of the system.
//For example, BoolConstWS is returning a constant boolean. If one types in
//true, the value true is created with index TYPE_INDEX_BOOL, the system is
//not required for this.
//On the other hand, IdentWS is used whenever an arbitrary identifier is 
//referred to.
//For an identifier x:
//IdentWS gets x from the system and evaluates it in the current context

namespace TransLucid
{
  class System;

  namespace Workshops
  {
    /**
     * @brief The outermost hyperdaton which starts an evaluation.
     * Sets up the right context so that evaluation works.
     */
    class SystemEvaluationWS : public WS
    {
      public:
      SystemEvaluationWS(System* system, WS* e)
      : m_system(system), m_e(e)
      {
      }

      Constant 
      operator()(Context& k);

      private:
      System* m_system;
      WS* m_e;
    };

    class HashSymbolWS : public WS
    {
      public:

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& k, Context& delta);
    };

    class DimensionWS : public WS
    {
      public:
      DimensionWS(System& system, const std::u32string& name);

      DimensionWS(System& system, dimension_index dim);

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& k, Context& delta);

      private:
      Constant m_value;
    };

    class IdentWS : public WS
    {
      public:
      IdentWS(const System::IdentifierLookup& idents, const u32string& name)
      : m_identifiers(idents), m_name(name), m_e(nullptr)
      {}

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& k, Context& delta);

      private:
      System::IdentifierLookup m_identifiers;
      u32string m_name;

      //don't delete this, it doesn't belong to you
      WS* m_e;

      template <typename... Delta>
      Constant
      evaluate(Context& kappa, Delta&&... delta);
    };

    class VariableOpWS : public WS
    {
      public:
      VariableOpWS(WS* system,
                const std::vector<WS*>& operands,
                const u32string& symbol)
      : m_system(system), m_operands(operands), m_symbol(symbol)
      {
      }

      Constant
      operator()(Context& k);

      private:
      WS* m_system;
      const std::vector<WS*>& m_operands;
      u32string m_symbol;
    };

    /**
     * A bang operation workshop. Evaluates a host function.
     */
    class BangOpWS : public WS
    {
      public:
      /**
       * Create a bang operation workshop.
       * @param system The system that we are evaluating in.
       * @param name The workshop that evaluates to the name of the function.
       * @param args The arguments to the function.
       * @return The result of evaluating the bang expression.
       * @todo Change name to @a e because it should evaluate to an abstraction
       * now.
       */
      BangOpWS
      (
        System& system, 
        WS* name,
        const std::vector<WS*>& args
      )
      : m_system(system)
      , m_name(name)
      , m_args(args)
      , m_numArgs(args.size())
      {
      }

      ~BangOpWS()
      {
        delete m_name;
        for (auto w : m_args)
        {
          delete w;
        }
      }

      /**
       * Evaluate the bang application. The expression either evaluates to a
       * name or a bang abstraction. It looks up the function in the system if
       * it evaluates to a name.
       * @todo It shouldn't evaluate to a name anymore.
       * @param k The current context.
       * @return The result of evaluating the host function.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System& m_system;
      WS* m_name;
      std::vector<WS*> m_args;
      size_t m_numArgs;
    };

    class BangOpSingleWS : public WS
    {
    };

    class MakeIntenWS : public WS
    {
      public:

      MakeIntenWS
      (
        System& system,
        WS* rhs,
        std::vector<WS*> binds,
        const std::vector<dimension_index>& scope
      )
      : m_system(system), m_rhs(rhs), m_binds(binds), m_scope(scope)
      {
      }

      ~MakeIntenWS()
      {
        delete m_rhs;
      }

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System& m_system;
      WS* m_rhs;
      std::vector<WS*> m_binds;
      std::vector<dimension_index> m_scope;
    };

    class EvalIntenWS : public WS
    {
      public:

      EvalIntenWS(WS* rhs)
      : m_rhs(rhs)
      {
      }

      ~EvalIntenWS()
      {
        delete m_rhs;
      }

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      WS* m_rhs;
    };

    /**
     * An if expression workshop.
     */
    class IfWS : public WS
    {
      public:

      /**
       * Create the if expression.
       * @param condition The condition.
       * @param then Evaluate then if the condition is true.
       * @param elsifs Look through these and evaluate the first one that is
       * true if the condition is false.
       * @param else_ Evaluate else_ if everything else is false.
       */
      IfWS(WS* condition, WS* then,
        const std::vector<std::pair<WS*, WS*>>& elsifs,
        WS* else_)
      : m_condition(condition),
        m_then(then),
        m_elsifs_2(elsifs),
        m_else(else_)
      {}

      ~IfWS();

      /**
       * Evaluates the if expression.
       * @param k The current context.
       * @return The result of the first expression whose condition evaluates
       * to true, the result of the else if none. A special if a condition
       * doesn't evaluate to a boolean.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      //WS* m_system;
      WS* m_condition;
      WS* m_then;
      std::vector<std::pair<WS*, WS*>> m_elsifs_2;
      WS* m_else;
    };

    /**
     * The hash workshop. When the code @b #!E appears, it is translated to a
     * HashWS. Looks up the evaluation of E in the current context.
     */
    class HashWS : public WS
    {
      public:
      /**
       * Creates a hash workshop.
       * @param system The system that we are working with.
       * @param e The expression to evaluate and lookup the result of.
       */
      HashWS(System& system, WS* e, bool cached = true)
      : m_system(system), m_e(e), m_cached(cached)
      {}

      ~HashWS()
      {
        delete m_e;
      }

      /**
       * Evaluate a hash expression. Evaluates @a e and looks up the result in
       * the current context.
       * @param k The current context.
       * @return The result of looking up the result of @a e in the current
       * context.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System& m_system;
      WS* m_e;
      bool m_cached;
    };

    class HostOpWS : public WS
    {
      public:
      HostOpWS(System& s, const u32string& name)
      : m_system(s), m_name(name), m_function(nullptr)
      {
      }

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System& m_system;
      u32string m_name;
      BaseFunctionType* m_function;
    };

    /**
     * A tuple creation workshop.
     */
    class TupleWS : public WS
    {
      public:

      /**
       * Creates the workshop.
       * @param system The system that we are working with.
       * @param elements The pairs of tuple elements.
       */
      TupleWS(System& system,
                 const std::list<std::pair<WS*, WS*>>& elements)
      : m_system(system), m_elements(elements)
      {}

      ~TupleWS()
      {
        for (auto& p : m_elements)
        {
          delete p.first;
          delete p.second;
        }
      }

      /**
       * Evaluates the tuple. Creates a physical tuple object.
       * @param k The current context.
       * @return The created tuple or the highest valued special if one of the
       * dimensions evaluates to a special.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System& m_system;
      std::list<std::pair<WS*, WS*>> m_elements;

      public:
      /**
       * Gets the pairs in a tuple.
       * @return A list of the pairs.
       */
      auto
      getElements() const ->
      const decltype(m_elements)&
      {
        return m_elements;
      }

      void
      releaseElements()
      {
        m_elements.clear();
      }

      /**
       * Get the system that the tuple workshop was created with.
       * @return The system.
       */
      System&
      getSystem()
      {
        return m_system;
      }
    };

    /**
     * An at expression workshop. Evaluates an at expression.
     */
    class AtWS : public WS
    {
      public:

      /**
       * Creates the at expression workshop.
       * @param e1 The tuple to change the context to.
       * @param e2 The expression to evaluate in the changed context.
       */
      AtWS(WS* e2, WS* e1)
      : e2(e2), e1(e1)
      {}

      ~AtWS()
      {
        delete e1;
        delete e2;
      }

      /**
       * Evaluates the at expression. First evaluates e1, if it is a tuple,
       * evaluates e2 in the changed context.
       * @param k The current context.
       * @return The result. The special @c spconst if e1 does not evaluate to 
       * a tuple.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      WS* e2;
      WS* e1;
    };

    /**
     * A base function abstraction workshop.
     * Creates a base function abstraction.
     */
    class BaseAbstractionWS : public WS
    {
      public:
      /**
       * Creates the workshop.
       * @param name The argument name.
       * @param dim The argument dimension.
       * @param scope The functions that the abstraction is in the scope of.
       * @param rhs The function definition.
       */
      BaseAbstractionWS
      (
        System* system,
        const std::vector<dimension_index>& dims,
        const std::vector<dimension_index>& scope,
        const std::vector<WS*>& binds,
        WS* rhs
      )
      : m_system(system)
      , m_dims(dims)
      , m_scope(scope)
      , m_binds(binds)
      , m_rhs(rhs)
      {
        if (rhs == nullptr)
        {
          std::cerr << "base abstraction workshop with nullptr body" 
            << std::endl;
        }
      }

      ~BaseAbstractionWS()
      {
        delete m_rhs;
      }

      /**
       * Evaluates the abstraction.
       * Creates a base abstraction object. The whole context is
       * bound.
       * @param k The current context.
       * @return The base abstraction.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System* m_system;
      std::vector<dimension_index> m_dims;
      std::vector<dimension_index> m_scope;
      std::vector<WS*> m_binds;
      WS* m_rhs;
    };

    /**
     * A call-by-value abstraction workshop.
     * Creates a call-by-value abstraction.
     */
    class LambdaAbstractionWS : public WS
    {
      public:
      /**
       * Creates the workshop.
       * @param name The paramater name.
       * @param dim The abstraction's argument dimension.
       * @param scope The dimensions of the functions that it is in the scope
       * of.
       * @param rhs The function definition.
       */
      LambdaAbstractionWS
      (
        System* system,
        const u32string& name, 
        dimension_index dim, 
        WS* rhs,
        const std::vector<WS*>& binds,
        const std::vector<dimension_index>& scope
      )
      : m_system(system)
      , m_name(name)
      , m_argDim(dim)
      , m_rhs(rhs)
      , m_binds(binds)
      , m_scope(scope)
      {
      }

      ~LambdaAbstractionWS()
      {
        delete m_rhs;
      }

      void
      set_rhs(WS* rhs)
      {
        delete m_rhs;
        m_rhs = rhs;
      }

      WS*
      rhs() const
      {
        return m_rhs;
      }

      /**
       * Evaluates the abstraction.
       * Creates a call-by-value abstraction.
       * @param k The current context.
       * @return The call-by-value abstraction.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      System* m_system;
      u32string m_name;
      dimension_index m_argDim;
      WS* m_rhs;
      std::vector<WS*> m_binds;
      std::vector<dimension_index> m_scope;
    };

    /*
     * A call-by-value application workshop.
     * Applies an argument to a call-by-value function.
     */
    class LambdaApplicationWS : public WS
    {
      public:
      /**
       * Creates the workshop.
       * @param lhs The lhs argument.
       * @param rhs The rhs argument.
       */
      LambdaApplicationWS(WS* lhs, WS* rhs)
      : m_lhs(lhs)
      , m_rhs(rhs)
      {
      }

      ~LambdaApplicationWS()
      {
        delete m_lhs;
        delete m_rhs;
      }

      /**
       * Evaluates the application. If the lhs evaluates to a call-by-value
       * abstraction, the rhs is evaluated and the abstraction applied to it. 
       * @param k The current context.
       * @return The result of evaluating the function with the supplied
       * argument. If lhs does not evaluate to a call-by-value abstraction,
       * then @c spconst.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      WS* m_lhs;
      WS* m_rhs;
    };

    class WhereWS : public WS
    {
      public:
      WhereWS
      (
        WS* expr,
        std::vector<std::pair<int, WS*>> dims,
        System& system
      )
      : m_expr(expr)
      , m_dims(dims)
      , m_system(system)
      {
      }

      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:

      WS* m_expr;
      std::vector<std::pair<int, WS*>> m_dims;

      System& m_system;
    };

    /**
     * Evaluates an at expression when the rhs is a tuple expression. This
     * allows us to make an optimisation by not building tuples all the time.
     */
    class AtTupleWS : public WS
    {
      public:
      template <typename T>
      AtTupleWS
      (
        WS* e2,
        const T& pairs,
        System& system
      )
      : m_e2(e2)
      , m_tuple(pairs.begin(), pairs.end())
      , m_system(system)
      {
      }

      ~AtTupleWS();

      /**
       * Evaluate @a e2 after changing the context to @a e1. This is done more
       * efficiently than building a tuple, it is done by evaluating the
       * context change in place.
       * @param k The current context.
       * @return The result of @a e2 in the context perturbed by @e1.
       */
      Constant
      operator()(Context& k);

      Constant
      operator()(Context& kappa, Context& delta);

      private:
      WS* m_e2;
      std::vector<std::pair<WS*, WS*>> m_tuple;
      System& m_system;
    };
  }
}

#endif // TL_EVAL_WORKSHOP_HPP_INCLUDED
