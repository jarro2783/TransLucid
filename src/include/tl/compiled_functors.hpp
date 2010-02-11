#ifndef SET_EVALUATOR_HPP_INCLUDED
#define SET_EVALUATOR_HPP_INCLUDED

#include <tl/interpreter.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/builtin_types.hpp>

namespace TransLucid
{

  namespace CompiledFunctors
  {

    class CompiledFunctor : public HD
    {
      void
      addExpr(const Tuple& k, HD* h)
      {
      }
    };

    //e2 @ e1
    class AtAbsolute : public CompiledFunctor
    {
      public:

      AtAbsolute(Interpreter& system, HD* e2, HD* e1)
      : m_system(system), e2(e2), e1(e1)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      HD* e2;
      HD* e1;
    };

    class AtRelative : public CompiledFunctor
    {
      public:

      AtRelative(Interpreter& system, HD* e2, HD* e1)
      : m_system(system), e2(e2), e1(e1)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      HD* e2;
      HD* e1;
    };

    class BinaryOp : public CompiledFunctor
    {
      public:

      BinaryOp(const std::vector<HD*>& operands)
      : operands(operands)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      std::vector<HD*> operands;
    };

    class BoolConst : public CompiledFunctor
    {
      public:

      BoolConst(Interpreter& system, bool value)
      : m_system(system), m_value(value)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      bool m_value;
    };

    class BuildTuple : public CompiledFunctor
    {
      public:

      BuildTuple(Interpreter& system,
                 const std::list<std::pair<HD*, HD*>>& elements)
      : m_system(system), m_elements(elements)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      std::list<std::pair<HD*, HD*>> m_elements;
    };

    class Constant : public CompiledFunctor
    {
      public:

      //typedef RawType some_crazy_MPL_thing;

      Constant(Interpreter& system,
               const std::u32string& type, const std::u32string& text)
      : m_system(system), m_type(type), m_text(text)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      std::u32string m_type;
      std::u32string m_text;
    };

    class Convert : public CompiledFunctor
    {
      public:
      Convert(const ustring_t& to, HD* e)
      : m_to(to), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      ustring_t m_to;
      HD* m_e;
    };

    class Dimension : public CompiledFunctor
    {
      public:
      Dimension(Interpreter& system, const std::u32string& name)
      : m_system(system), m_name(name)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
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
      If(Interpreter& system, HD* condition, HD* then,
        const std::list<HD*>& elsifs,
        HD* else_)
      : m_system(system),
        m_condition(condition),
        m_then(then),
        m_elsifs(elsifs),
        m_else(else_)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      HD* m_condition;
      HD* m_then;
      std::list<HD*> m_elsifs;
      HD* m_else;
    };

    class Integer : public CompiledFunctor
    {
      public:
      Integer(Interpreter& system, const mpz_class& value)
      : m_system(system), m_value(value)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      mpz_class m_value;
    };

    class IsSpecial : public CompiledFunctor
    {
      public:
      IsSpecial(const ustring_t& special, HD* e)
      : m_special(special),
      m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      ustring_t m_special;
      HD* m_e;
    };

    class IsType : public CompiledFunctor
    {
      public:
      IsType(const ustring_t& type, HD* e)
      : m_type(type), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      ustring_t m_type;
      HD* m_e;
    };

    class Operation : public CompiledFunctor
    {
      public:
      Operation(Interpreter& i,
                const std::vector<HD*>& operands,
                const ustring_t& symbol)
      : m_system(i), m_operands(operands), m_symbol(symbol)
      {
      }

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
      const std::vector<HD*>& m_operands;
      ustring_t m_symbol;
    };

    class Pair : public CompiledFunctor
    {
      public:
      Pair(Interpreter& system, HD* lhs, HD* rhs)
      : m_system(system), m_lhs(lhs), m_rhs(rhs)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Interpreter& m_system;
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
      UnaryOp(Parser::UnaryOperation op, HD* e)
      : m_op(op), m_e(e)
      {}

      TaggedValue
      operator()(const Tuple& context);

      private:
      Parser::UnaryOperation m_op;
      HD* m_e;
    };

  }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
