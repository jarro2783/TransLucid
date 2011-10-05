/* Workshops generated from AST::Expr.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#ifndef SET_EVALUATOR_HPP_INCLUDED
#define SET_EVALUATOR_HPP_INCLUDED

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

//Some of these functors are constructed with the system, some are not.
//The ones that are make a demand of the system.
//For example, BoolConstWS is returning a constant boolean. If one types in
//true, the value true is created with index TYPE_INDEX_BOOL, the system is
//not required for this.
//On the other hand, IdentWS is used whenever an arbitrary identifier is 
//referred to.
//For an identifier x:
//IdentWS makes the demand system([id : "x"]) & k

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

    class DimensionWS : public WS
    {
      public:
      DimensionWS(System& system, const std::u32string& name);

      DimensionWS(System& system, dimension_index dim);

      Constant
      operator()(Context& k);

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

      private:
      System::IdentifierLookup m_identifiers;
      u32string m_name;

      //don't delete this, it doesn't belong to you
      WS* m_e;
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

    class IsSpecialWS : public WS
    {
      public:
      IsSpecialWS(WS* system, const u32string& special, WS* e)
      : m_special(special),
      m_e(e)
      {}

      Constant
      operator()(Context& k);

      private:
      WS* m_system;
      u32string m_special;
      WS* m_e;
    };

    class IsTypeWS : public WS
    {
      public:
      IsTypeWS(WS* system, const u32string& type, WS* e)
      : m_system(system), m_type(type), m_e(e)
      {}

      Constant
      operator()(Context& k);

      private:
      WS* m_system;
      u32string m_type;
      WS* m_e;
    };

    //TODO work out what converting is
    #if 0
    class ConvertWS : public WS
    {
      public:
      ConvertWS(const u32string& to, WS* e)
      : m_to(to), m_e(e)
      {}

      Constant
      operator()(const Tuple& k);

      private:
      u32string m_to;
      WS* m_e;
    };
    #endif

    typedef Constant (*BangFunc)
    (
      System& system,
      const u32string&,
      const std::vector<WS*>&, 
      Context&
    );

    //N arguments, but they go from 0 to N-1, so evaluate the (N-1)th
    template <size_t N>
    struct evaluate_bang_func
    {
      template <typename F, typename... Args>
      Constant
      operator()
      (
        F f, 
        const std::vector<WS*>& uneval, 
        Context& k,
        Args... args
      )
      {
        return evaluate_bang_func<N-1>()
          (f, uneval, k, (*uneval[N-1])(k), args...);
      }
    };

    template <>
    struct evaluate_bang_func<0>
    {
      template <typename F, typename... Args>
      Constant
      operator()
      (
        F f,
        const std::vector<WS*>& uneval,
        Context& k,
        Args... args
      )
      {
        return f(args...);
      }
    };

    template <size_t N>
    Constant
    bang_func
    (
      System& system,
      const u32string& name,
      const std::vector<WS*>& args,
      Context& k
    )
    {
      auto f = system.lookupFunction<N>(name);

      if (f)
      {
        return evaluate_bang_func<N>()(f, args, k);
      }
      else
      {
        return Types::Special::create(SP_UNDEF);
      }
    }

    template <size_t N>
    struct generate_bang_funcs
    {
      generate_bang_funcs(BangFunc* funcs)
      {
        funcs[N] = &bang_func<N>;
        generate_bang_funcs<N-1> tmp(funcs);
      }
    };

    template <>
    struct generate_bang_funcs<-1>
    {
      generate_bang_funcs(BangFunc* funcs)
      {
      }
    };

    template <size_t N>
    class BangCaller
    {
      public:

      BangCaller(System& system)
      : m_system(system)
      {
        generate_bang_funcs<N> tmp(m_funcs);
      }

      Constant
      operator()
      (
        const u32string& name, 
        const std::vector<WS*>& args,
        Context& k
      )
      {
        if (args.size() < N)
        {
          return (*m_funcs[args.size()])(m_system, name, args, k);
        }
        else
        {
          return Types::Special::create(SP_UNDEF);
        }
      }

      private:
      System& m_system;

      //N+1 spots for 0 to N parameters
      BangFunc m_funcs[N+1];
    };

    class BangOpWS : public WS
    {
      public:
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
      , m_caller(system)
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

      Constant
      operator()(Context& k);

      private:
      System& m_system;
      WS* m_name;
      std::vector<WS*> m_args;
      size_t m_numArgs;
      BangCaller<MAX_FUNCTION_PARAMETERS> m_caller;
    };

    class BangOpSingleWS : public WS
    {
    };

    class IfWS : public WS
    {
      public:

      IfWS(WS* condition, WS* then,
        const std::vector<std::pair<WS*, WS*>>& elsifs,
        WS* else_)
      : m_condition(condition),
        m_then(then),
        m_elsifs_2(elsifs),
        m_else(else_)
      {}

      Constant
      operator()(Context& k);

      private:
      //WS* m_system;
      WS* m_condition;
      WS* m_then;
      std::list<WS*> m_elsifs;
      std::vector<std::pair<WS*, WS*>> m_elsifs_2;
      WS* m_else;
    };

    class HashWS : public WS
    {
      public:
      HashWS(System& system, WS* e)
      : m_system(system), m_e(e)
      {}

      ~HashWS()
      {
        delete m_e;
      }

      Constant
      operator()(Context& k);

      private:
      System& m_system;
      WS* m_e;
    };

#if 0
    //TODO: What is this?
    class PairWS : public WS
    {
      public:
      PairWS(WS* lhs, WS* rhs)
      : m_lhs(lhs), m_rhs(rhs)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_lhs;
      WS* m_rhs;
    };
#endif

    class TupleWS : public WS
    {
      public:

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

      Constant
      operator()(Context& k);

      private:
      System& m_system;
      std::list<std::pair<WS*, WS*>> m_elements;

      public:
      auto
      getElements() const ->
      const decltype(m_elements)&
      {
        return m_elements;
      }

      System&
      getSystem()
      {
        return m_system;
      }
    };

    class AtWS : public WS
    {
      public:

      AtWS(WS* e2, WS* e1)
      : e2(e2), e1(e1)
      {}

      ~AtWS()
      {
        delete e1;
        delete e2;
      }

      Constant
      operator()(Context& k);

      private:
      WS* e2;
      WS* e1;
    };

    struct FunctionInfo
    {
      std::vector<dimension_index> valueScopeArgs;
      std::vector<dimension_index> namedScopeArgs;
      std::vector<dimension_index> namedScopeOdometers;
    };

    class LambdaAbstractionWS : public WS
    {
      public:
      LambdaAbstractionWS
      (
        const u32string& name, 
        dimension_index dim, 
        std::vector<dimension_index> valueScopeArgs,
        std::vector<dimension_index> namedScopeArgs,
        std::vector<dimension_index> namedScopeOdometers,
        WS* rhs
      )
      : m_name(name)
      , m_argDim(dim)
      , m_info{valueScopeArgs, namedScopeArgs, namedScopeOdometers}
      , m_rhs(rhs)
      {
      }

      ~LambdaAbstractionWS()
      {
        delete m_rhs;
      }

      Constant
      operator()(Context& k);

      private:
      WS* m_system;
      u32string m_name;
      dimension_index m_argDim;
      FunctionInfo m_info;
      WS* m_rhs;
    };

    class LambdaApplicationWS : public WS
    {
      public:
      LambdaApplicationWS(WS* lhs, WS* rhs)
      : m_lhs(lhs)
      , m_rhs(rhs)
      {
      }

      Constant
      operator()(Context& k);

      private:
      WS* m_lhs;
      WS* m_rhs;
    };

    class NamedAbstractionWS : public WS
    {
      public:
      NamedAbstractionWS
      (
        const u32string& name,
        dimension_index argDim,
        dimension_index odometerDim,
        std::vector<dimension_index> valueScopeArgs,
        std::vector<dimension_index> namedScopeArgs,
        std::vector<dimension_index> namedScopeOdometers,
        WS* rhs
      )
      : m_name(name)
      , m_argDim(argDim)
      , m_odometerDim(odometerDim)
      , m_info{valueScopeArgs, namedScopeArgs, namedScopeOdometers}
      , m_rhs(rhs)
      {
      }

      ~NamedAbstractionWS()
      {
        delete m_rhs;
      }

      Constant
      operator()(Context& k);

      private:
      u32string m_name;
      dimension_index m_argDim;
      dimension_index m_odometerDim;
      FunctionInfo m_info;
      WS* m_rhs;
    };

    class NameApplicationWS : public WS
    {
      public:
      NameApplicationWS(WS* lhs, WS* rhs)
      : m_lhs(lhs)
      , m_rhs(rhs)
      {
      }

      Constant
      operator()(Context& k);

      private:
      WS* m_lhs;
      WS* m_rhs;

      std::vector<dimension_index> m_Lall;
    };

  }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
