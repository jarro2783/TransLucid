#ifndef SET_EVALUATOR_HPP_INCLUDED
#define SET_EVALUATOR_HPP_INCLUDED

#include <tl/interpreter.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid {

   namespace SetLazyEvaluator {

      class AtAbsolute : public HD {
         public:

         AtAbsolute(HD *e2, HD *e1)
         : e2(e2), e1(e1)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         HD *e2;
         HD *e1;
      };

      class AtRelative : public HD {
         public:

         AtRelative(HD *e2, HD *e1)
         : e2(e2), e1(e1)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         HD *e2;
         HD *e1;
      };

      class BinaryOp : public HD {
         public:

         BinaryOp(const std::vector<HD*>& operands)
         : operands(operands)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         std::vector<HD*> operands;
      };

      class Boolean : public HD {
         public:

         Boolean(bool value)
         : m_value(value)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         bool m_value;
      };

      class BuildTuple : public HD {
         public:

         BuildTuple(const std::list<HD*>& elements)
         : m_elements(elements)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         std::list<HD*> m_elements;
      };

      class Constant : public HD {
         public:

         //typedef RawType some_crazy_MPL_thing;

         Constant(const ustring_t& name, const ustring_t& value)
         : m_name(name), m_value(value)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         ustring_t m_name;
         ustring_t m_value;
      };

      class Convert : public HD {
         public:
         Convert(const ustring_t& to, HD *e)
         : m_to(to), m_e(e)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         ustring_t m_to;
         HD *m_e;
      };

      class Dimension : public HD {
         public:
         Dimension(const ustring_t& name)
         : m_name(name)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         ustring_t m_name;
      };

      class Hash : public HD {
         public:
         Hash(HD *e)
         : m_e(e)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         HD *m_e;
      };

      class Ident : public HD {
         public:
         Ident(const ustring_t& name)
         : m_name(name)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         ustring_t m_name;
      };

      class If : public HD {
         public:
         If(HD *condition, HD *then,
            const std::list<HD*>& elseifs,
            HD *else_)
         : m_condition(condition),
         m_then(then),
         m_elseifs(elseifs),
         m_else(else_)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         HD *m_condition;
         HD *m_then;
         std::list<HD*> m_elseifs;
         HD *m_else;
      };

      class Integer : public HD {
         public:
         Integer(const mpz_class& value)
         : m_value(value)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         mpz_class m_value;
      };

      class IsSpecial : public HD {
         public:
         IsSpecial(const ustring_t& special, HD *e)
         : m_special(special),
         m_e(e)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         ustring_t m_special;
         HD *m_e;
      };

      class IsType : public HD {
         public:
         IsType(const ustring_t& type, HD *e)
         : m_type(type), m_e(e)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         ustring_t m_type;
         HD *m_e;
      };

      class Pair : public HD {
         public:
         Pair(HD *lhs, HD *rhs)
         : m_lhs(lhs), m_rhs(rhs)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         HD *m_lhs;
         HD *m_rhs;
      };

      class UnaryOp : public HD {
         public:
         UnaryOp(Parser::UnaryOperation op, HD *e)
         : m_op(op), m_e(e)
         {}

         TaggedValue operator()(const Tuple& context);

         private:
         Parser::UnaryOperation m_op;
         HD *m_e;
      };

   }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
