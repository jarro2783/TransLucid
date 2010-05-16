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

#include <tl/hyperdaton.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/builtin_types.hpp>
#include <tl/ast.hpp>

namespace TransLucid
{
  namespace Hyperdatons
  {
    class BoolHD : public HD
    {
      public:

      BoolHD(bool value)
      : m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      bool m_value;
    };

    class SpecialHD : public HD
    {
      public:
      SpecialHD(Special::Value v)
      : m_value(v)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      Special::Value m_value;
    };

    class IntmpConstHD : public HD
    {
      public:
      IntmpConstHD(HD* system, const mpz_class& value)
      : m_system(system), m_value(value)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      mpz_class m_value;
    };

    class UCharConstHD : public HD
    {
      public:
      UCharConstHD(char32_t c)
      : m_value(c)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      char32_t m_value;
    };

    class UStringConstHD : public HD
    {
      public:
      UStringConstHD(const u32string& s)
      : m_value(s)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      u32string m_value;
    };

    class TypedValueHD : public HD
    {
      public:

      TypedValueHD(HD* system,
               const std::u32string& type, const std::u32string& text)
      : m_system(system), m_type(type), m_text(text)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      std::u32string m_type;
      std::u32string m_text;
    };

    class DimensionHD : public HD
    {
      public:
      DimensionHD(HD* system, const std::u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      std::u32string m_name;
    };

    class IdentHD : public HD
    {
      public:
      IdentHD(HD* system, const u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      u32string m_name;
    };

    class UnaryOpHD : public HD
    {
      public:
      UnaryOpHD(Tree::UnaryOperation op, HD* e)
      : m_op(op), m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      Tree::UnaryOperation m_op;
      HD* m_e;
    };

    class BinaryOpHD : public HD
    {
      public:

      BinaryOpHD
      (
        HD* system,
        const std::vector<HD*>& operands,
        const u32string& name
      )
      : m_system(system), m_operands(operands), m_name(name)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      std::vector<HD*> m_operands;
      u32string m_name;
    };

    class OperationHD : public HD
    {
      public:
      OperationHD(HD* system,
                const std::vector<HD*>& operands,
                const u32string& symbol)
      : m_system(system), m_operands(operands), m_symbol(symbol)
      {
      }

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      const std::vector<HD*>& m_operands;
      u32string m_symbol;
    };

    class IsSpecialHD : public HD
    {
      public:
      IsSpecialHD(const u32string& special, HD* e)
      : m_special(special),
      m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      u32string m_special;
      HD* m_e;
    };

    class IsTypeHD : public HD
    {
      public:
      IsTypeHD(const u32string& type, HD* e)
      : m_type(type), m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      u32string m_type;
      HD* m_e;
    };

    class ConvertHD : public HD
    {
      public:
      ConvertHD(const u32string& to, HD* e)
      : m_to(to), m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      u32string m_to;
      HD* m_e;
    };

    class IfHD : public HD
    {
      public:

      IfHD(HD* condition, HD* then,
        const std::vector<std::pair<HD*, HD*>>& elsifs,
        HD* else_)
      : m_condition(condition),
        m_then(then),
        m_elsifs_2(elsifs),
        m_else(else_)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      //HD* m_system;
      HD* m_condition;
      HD* m_then;
      std::list<HD*> m_elsifs;
      std::vector<std::pair<HD*, HD*>> m_elsifs_2;
      HD* m_else;
    };

    class HashHD : public HD
    {
      public:
      HashHD(HD* system, HD* e)
      : m_system(system), m_e(e)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      HD* m_e;
    };

#if 0
    //TODO: What is this?
    class PairHD : public HD
    {
      public:
      PairHD(HD* lhs, HD* rhs)
      : m_lhs(lhs), m_rhs(rhs)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_lhs;
      HD* m_rhs;
    };
#endif

    class TupleHD : public HD
    {
      public:

      TupleHD(HD* system,
                 const std::list<std::pair<HD*, HD*>>& elements)
      : m_system(system), m_elements(elements)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* m_system;
      std::list<std::pair<HD*, HD*>> m_elements;
    };

    //e2 @ e1
    class AtAbsoluteHD : public HD
    {
      public:

      AtAbsoluteHD(HD* e2, HD* e1)
      : e2(e2), e1(e1)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* e2;
      HD* e1;
    };

    class AtRelativeHD : public HD
    {
      public:

      AtRelativeHD(HD* e2, HD* e1)
      : e2(e2), e1(e1)
      {}

      TaggedConstant
      operator()(const Tuple& k);

      private:
      HD* e2;
      HD* e1;
    };

  }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
