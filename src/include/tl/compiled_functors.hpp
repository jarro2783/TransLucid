/* TODO: Give a descriptor.
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

  namespace CompiledFunctors
  {

    class CompiledFunctor : public HD
    {
    };

    //e2 @ e1
    class AtAbsolute : public CompiledFunctor
    {
      public:

      AtAbsolute(HD* e2, HD* e1)
      : e2(e2), e1(e1)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* e2;
      HD* e1;
    };

    class AtRelative : public CompiledFunctor
    {
      public:

      AtRelative(HD* e2, HD* e1)
      : e2(e2), e1(e1)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* e2;
      HD* e1;
    };

    class BinaryOp : public CompiledFunctor
    {
      public:

      BinaryOp
      (
        HD* system,
        const std::vector<HD*>& operands,
        const u32string& name
      )
      : m_system(system), m_operands(operands), m_name(name)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      std::vector<HD*> m_operands;
      u32string m_name;
    };

    class BoolConst : public CompiledFunctor
    {
      public:

      BoolConst(bool value)
      : m_value(value)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      bool m_value;
    };

    class BuildTuple : public CompiledFunctor
    {
      public:

      BuildTuple(HD* system,
                 const std::list<std::pair<HD*, HD*>>& elements)
      : m_system(system), m_elements(elements)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      std::list<std::pair<HD*, HD*>> m_elements;
    };

    class Constant : public CompiledFunctor
    {
      public:

      Constant(HD* system,
               const std::u32string& type, const std::u32string& text)
      : m_system(system), m_type(type), m_text(text)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      std::u32string m_type;
      std::u32string m_text;
    };

    class Convert : public CompiledFunctor
    {
      public:
      Convert(const u32string& to, HD* e)
      : m_to(to), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      u32string m_to;
      HD* m_e;
    };

    class Dimension : public CompiledFunctor
    {
      public:
      Dimension(HD* system, const std::u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      std::u32string m_name;
    };

    class Hash : public CompiledFunctor
    {
      public:
      Hash(HD* system, HD* e)
      : m_system(system), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      HD* m_e;
    };

    class Ident : public CompiledFunctor
    {
      public:
      Ident(HD* system, const u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      u32string m_name;
    };

    class If : public CompiledFunctor
    {
      public:
      If(HD* condition, HD* then,
        const std::list<HD*>& elsifs,
        HD* else_)
      : m_condition(condition),
        m_then(then),
        m_elsifs(elsifs),
        m_else(else_)
      {}

      If(HD* condition, HD* then,
        const std::vector<std::pair<HD*, HD*>>& elsifs,
        HD* else_)
      : m_condition(condition),
        m_then(then),
        m_elsifs_2(elsifs),
        m_else(else_)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      //HD* m_system;
      HD* m_condition;
      HD* m_then;
      std::list<HD*> m_elsifs;
      std::vector<std::pair<HD*, HD*>> m_elsifs_2;
      HD* m_else;
    };

    class Integer : public CompiledFunctor
    {
      public:
      Integer(HD* system, const mpz_class& value)
      : m_system(system), m_value(value)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      mpz_class m_value;
    };

    class IsSpecial : public CompiledFunctor
    {
      public:
      IsSpecial(const u32string& special, HD* e)
      : m_special(special),
      m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      u32string m_special;
      HD* m_e;
    };

    class IsType : public CompiledFunctor
    {
      public:
      IsType(const u32string& type, HD* e)
      : m_type(type), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      u32string m_type;
      HD* m_e;
    };

    class Operation : public CompiledFunctor
    {
      public:
      Operation(HD* system,
                const std::vector<HD*>& operands,
                const u32string& symbol)
      : m_system(system), m_operands(operands), m_symbol(symbol)
      {
      }

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_system;
      const std::vector<HD*>& m_operands;
      u32string m_symbol;
    };

    class Pair : public CompiledFunctor
    {
      public:
      Pair(HD* lhs, HD* rhs)
      : m_lhs(lhs), m_rhs(rhs)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      HD* m_lhs;
      HD* m_rhs;
    };

    class SpecialConst : public CompiledFunctor
    {
      public:
      SpecialConst(Special::Value v)
      : m_value(v)
      {}

      TaggedValue
      operator()(const Tuple& k);

      private:
      Special::Value m_value;
    };

    class StringConst : public CompiledFunctor
    {
      public:
      StringConst(const u32string& s)
      : m_value(s)
      {}

      TaggedValue
      operator()(const Tuple& k);

      private:
      u32string m_value;
    };

    class UcharConst : public CompiledFunctor
    {
      public:
      UcharConst(char32_t c)
      : m_value(c)
      {}

      TaggedValue
      operator()(const Tuple& k);

      private:
      char32_t m_value;
    };

    class UnaryOp : public CompiledFunctor
    {
      public:
      UnaryOp(Tree::UnaryOperation op, HD* e)
      : m_op(op), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Tree::UnaryOperation m_op;
      HD* m_e;
    };

  }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
