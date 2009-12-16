#ifndef SET_EVALUATOR_HPP_INCLUDED
#define SET_EVALUATOR_HPP_INCLUDED

#include <tl/interpreter.hpp>

namespace TransLucid {

   namespace SetLazyEvaluator {

      class AtAbsolute : public SetEvaluator {
         public:

         AtAbsolute(SetEvaluator *e2, SetEvaluator *e1)
         : e2(e2), e1(e1)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         SetEvaluator *e2;
         SetEvaluator *e1;
      };

      class AtRelative : public SetEvaluator {
         public:

         AtRelative(SetEvaluator *e2, SetEvaluator *e1)
         : e2(e2), e1(e1)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         SetEvaluator *e2;
         SetEvaluator *e1;
      };

      class BinaryOp : public SetEvaluator {
         public:

         BinaryOp(const std::vector<SetEvaluator*>& operands)
         : operands(operands)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         std::vector<SetEvaluator*> operands;
      };

      class Boolean : public SetEvaluator {
         public:

         Boolean(bool value)
         : m_value(value)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         bool m_value;
      };

      class BuildTuple : public SetEvaluator {
         public:

         BuildTuple(const std::list<SetEvaluator*>& elements)
         : m_elements(elements)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         std::list<SetEvaluator*> m_elements;
      };

      class Constant : public SetEvaluator {
         public:

         Constant(const ustring_t& name, const ustring_t& value)
         : m_name(name), m_value(value)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         ustring_t m_name;
         ustring_t m_value;
      };

      class Convert : public SetEvaluator {
         public:
         Convert(const ustring_t& to, SetEvaluator *e)
         : m_to(to), m_e(e)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         ustring_t m_to;
         SetEvaluator *m_e;
      };

      class Dimension : public SetEvaluator {
         public:
         Dimension(const ustring_t& name)
         : m_name(name)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         ustring_t m_name;
      };

      class Hash : public SetEvaluator {
         public:
         Hash(SetEvaluator *e)
         : m_e(e)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         SetEvaluator *m_e;
      };

      class Ident : public SetEvaluator {
         public:
         Ident(const ustring_t& name)
         : m_name(name)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         ustring_t m_name;
      };

      class If : public SetEvaluator {
         public:
         If(SetEvaluator *condition, SetEvaluator *then,
            const std::list<SetEvaluator*>& elseifs,
            SetEvaluator *else_)
         : m_condition(condition),
         m_then(then),
         m_elseifs(elseifs),
         m_else(else_)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         SetEvaluator *m_condition;
         SetEvaluator *m_then;
         std::list<SetEvaluator*> m_elseifs;
         SetEvaluator *m_else;
      };

      class Integer : public SetEvaluator {
         public:
         Integer(const mpz_class& value)
         : m_value(value)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         mpz_class m_value;
      };

      class IsSpecial : public SetEvaluator {
         public:
         IsSpecial(const ustring_t& special, SetEvaluator *e)
         : m_special(special),
         m_e(e)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         ustring_t m_special;
         SetEvaluator *m_e;
      };

      class IsType : public SetEvaluator {
         public:
         IsType(const ustring_t& type, SetEvaluator *e)
         : m_type(type), m_e(e)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         ustring_t m_type;
         SetEvaluator *m_e;
      };

      class Pair : public SetEvaluator {
         public:
         Pair(SetEvaluator *lhs, SetEvaluator *rhs)
         : m_lhs(lhs), m_rhs(rhs)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         SetEvaluator *m_lhs;
         SetEvaluator *m_rhs;
      };

      class UnaryOp : public SetEvaluator {
         public:
         UnaryOp(Parser::UnaryOperation op, SetEvaluator *e)
         : m_op(op), m_e(e)
         {}

         SetResult evaluate(const TupleSet& context, Interpreter& i);

         private:
         Parser::UnaryOperation m_op;
         SetEvaluator *m_e;
      };

   }

}

#endif // SET_EVALUATOR_HPP_INCLUDED
