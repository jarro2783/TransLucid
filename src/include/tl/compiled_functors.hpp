/* Hyperdatons generated from AST::Expr.
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

  namespace Hyperdatons
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

      TaggedConstant 
      operator()(const Tuple& k);

      private:
      System* m_system;
      WS* m_e;
    };

    class DimensionWS : public WS
    {
      public:
      DimensionWS(System& system, const std::u32string& name)
      : m_id(system.getDimensionIndex(name))
      {
      }

      TaggedConstant
      operator()(const Tuple& k);

      private:
      dimension_index m_id;
    };

    class IdentWS : public WS
    {
      public:
      IdentWS(const System::IdentifierLookup& idents, const u32string& name)
      : m_identifiers(idents), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      System::IdentifierLookup m_identifiers;
      u32string m_name;
    };

    class UnaryOpWS : public WS
    {
      public:
      UnaryOpWS
      (
        WS* system,
        u32string name,
        WS* e
      )
      : m_system(system), m_name(name), m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      u32string m_name;
      WS* m_e;
    };

    class BinaryOpWS : public WS
    {
      public:

      BinaryOpWS
      (
        WS* system,
        const std::vector<WS*>& operands,
        const u32string& name
      )
      : m_system(system), m_operands(operands), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      std::vector<WS*> m_operands;
      u32string m_name;
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

      TaggedConstant
      operator()(const Tuple& k);

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

      TaggedConstant
      operator()(const Tuple& k);

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

      TaggedConstant
      operator()(const Tuple& k);

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

      TaggedConstant
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
      const Tuple&
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
        const Tuple& k,
        Args... args
      )
      {
        return evaluate_bang_func<N-1>()
          (f, uneval, k, (*uneval[N-1])(k).first, args...);
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
        const Tuple& k,
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
      const Tuple& k
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

      TaggedConstant
      operator()
      (
        const u32string& name, 
        const std::vector<WS*>& args,
        const Tuple& k
      )
      {
        if (args.size() < N)
        {
          return TaggedConstant(
            (*m_funcs[args.size()])(m_system, name, args, k),
            k)
          ;
        }
        else
        {
          return TaggedConstant(Types::Special::create(SP_UNDEF), k);
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

      TaggedConstant
      operator()(const Tuple& k);

      private:
      System& m_system;
      WS* m_name;
      std::vector<WS*> m_args;
      size_t m_numArgs;
      BangCaller<MAX_FUNCTION_PARAMETERS> m_caller;
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

      TaggedConstant
      operator()(const Tuple& k);

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

      TaggedConstant
      operator()(const Tuple& k);

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

      TaggedConstant
      operator()(const Tuple& k);

      private:
      System& m_system;
      std::list<std::pair<WS*, WS*>> m_elements;
    };

    class AtWS : public WS
    {
      public:

      AtWS(WS* e2, WS* e1)
      : e2(e2), e1(e1)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* e2;
      WS* e1;
    };

    class LambdaAbstractionWS : public WS
    {
      public:
      LambdaAbstractionWS
      (
        WS* system, 
        const u32string& name, 
        dimension_index dim, 
        WS* rhs
      )
      : m_system(system)
      , m_name(name)
      , m_dim(dim)
      , m_rhs(rhs)
      {
      }

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      u32string m_name;
      dimension_index m_dim;
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

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_lhs;
      WS* m_rhs;
    };

  }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
