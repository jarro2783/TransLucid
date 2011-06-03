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
#include <tl/hyperdaton.hpp>

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

    class TypedValueWS : public WS
    {
      public:

      TypedValueWS(WS* system, const u32string& type, const u32string& text)
      : m_system(system), m_type(type), m_text(text)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      std::u32string m_type;
      std::u32string m_text;
    };

    class DimensionWS : public WS
    {
      public:
      DimensionWS(WS* system, const std::u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      std::u32string m_name;
    };

    class IdentWS : public WS
    {
      public:
      IdentWS(WS* system, const u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
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
      HashWS(WS* system, WS* e)
      : m_system(system), m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
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

      TupleWS(WS* system,
                 const std::list<std::pair<WS*, WS*>>& elements)
      : m_system(system), m_elements(elements)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      std::list<std::pair<WS*, WS*>> m_elements;
    };

    //e2 @ e1
    class AtAbsoluteWS : public WS
    {
      public:

      AtAbsoluteWS(WS* e2, WS* e1)
      : e2(e2), e1(e1)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* e2;
      WS* e1;
    };

    class AtRelativeWS : public WS
    {
      public:

      AtRelativeWS(WS* e2, WS* e1)
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
      LambdaApplicationWS(WS* system, WS* lhs, WS* rhs)
      : m_system(system)
      , m_lhs(lhs)
      , m_rhs(rhs)
      {
      }

      TaggedConstant
      operator()(const Tuple& k);

      private:
      WS* m_system;
      WS* m_lhs;
      WS* m_rhs;
    };

  }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
