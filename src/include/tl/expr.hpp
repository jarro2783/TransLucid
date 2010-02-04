#ifndef EXPR_HPP_INCLUDED
#define EXPR_HPP_INCLUDED

#include <tl/types.hpp>
#include <list>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <tl/parser_fwd.hpp>

namespace TransLucid {
   namespace AST {

      class Data {
         public:
         virtual ~Data() = 0;
      };

      inline Data::~Data() {}

      class Visitor;

      class Expr {
         public:
         virtual ~Expr() {}
         virtual Data *visit(Visitor *visitor, Data *data) = 0;
      };

      class AtExpr;
      class BinaryOpExpr;
      class BooleanExpr;
      class BuildTupleExpr;
      class ConstantExpr;
      class DimensionExpr;
      class HashExpr;
      class IdentExpr;
      class IfExpr;
      class IntegerExpr;
      class OpExpr;
      class PairExpr;
      class RangeExpr;
      class SpecialOpsExpr;
      class UnaryExpr;

      class Visitor {
         public:
         virtual ~Visitor() {}

         virtual Data *visitAtExpr(AtExpr*, Data*) = 0;
         virtual Data *visitBinaryOpExpr(BinaryOpExpr*, Data*) = 0;
         virtual Data *visitBooleanExpr(BooleanExpr*, Data*) = 0;
         virtual Data *visitBuildTupleExpr(BuildTupleExpr*, Data*) = 0;
         virtual Data *visitConstantExpr(ConstantExpr*, Data*) = 0;
         virtual Data *visitConvertExpr(SpecialOpsExpr*, Data*) = 0;
         virtual Data *visitDimensionExpr(DimensionExpr*, Data*) = 0;
         virtual Data *visitHashExpr(HashExpr*, Data*) = 0;
         virtual Data *visitIdentExpr(IdentExpr*, Data*) = 0;
         virtual Data *visitIfExpr(IfExpr*, Data*) = 0;
         virtual Data *visitIntegerExpr(IntegerExpr*, Data*) = 0;
         virtual Data *visitIsSpecialExpr(SpecialOpsExpr*, Data*) = 0;
         virtual Data *visitIsTypeExpr(SpecialOpsExpr*, Data*) = 0;
         virtual Data *visitOpExpr(OpExpr*, Data*) = 0;
         virtual Data *visitPairExpr(PairExpr*, Data*) = 0;
         virtual Data *visitRangeExpr(RangeExpr*, Data*) = 0;
         virtual Data *visitUnaryExpr(UnaryExpr*, Data*) = 0;
      };

      class AtExpr : public Expr {
         public:
         //e2 @ e1
         AtExpr(Expr *e2, Expr *e1)
         : e2(e2), e1(e1), relative(true)
         {
         }

         Data *visit(Visitor *v, Data *data) {
            return v->visitAtExpr(this, data);
         }

         Expr *e2;
         Expr *e1;
         bool relative;
      };

      class BinaryOpExpr : public Expr {

         public:
         BinaryOpExpr(const Parser::BinaryOperation& op, Expr *lhs, Expr *rhs)
         : op(op)
         {
            operands.push_back(lhs);
            operands.push_back(rhs);
         }

         Data *visit(Visitor *v, Data *data) {
            return v->visitBinaryOpExpr(this, data);
         }

         void add_right(const Parser::BinaryOperation& op, Expr *rhs);
         void add_leaf(Expr *e);

         Parser::BinaryOperation op;
         std::vector<Expr*> operands;
      };

      class BooleanExpr : public Expr {
         public:
         BooleanExpr(const ustring_t& value)
         : value(value == "true")
         {
         }

         Data *visit(Visitor *v, Data *data) {
            return v->visitBooleanExpr(this, data);
         }

         bool value;
      };

      class BuildTupleExpr : public Expr {
         public:

         BuildTupleExpr(const std::list<AST::Expr*>& l)
         : values(l)
         {
         }

         Data *visit(Visitor *v, Data *data) {
            return v->visitBuildTupleExpr(this, data);
         }

         std::list<AST::Expr*> values;
      };

      class ConstantExpr : public Expr {
         public:
         ConstantExpr(const ustring_t& name, const ustring_t& value)
         : name(name), value(value)
         {
         }

         Data *visit(Visitor *v, Data *data) {
            return v->visitConstantExpr(this, data);
         }

         ustring_t name;
         ustring_t value;
      };

      class DimensionExpr : public Expr {
         public:

         DimensionExpr(const ustring_t& v)
         : value(v)
         {
         }

         Data *visit(Visitor *v, Data *d) {
            return v->visitDimensionExpr(this, d);
         }

         ustring_t value;
      };

      class HashExpr : public Expr {
         public:
         HashExpr(Expr *e)
         : e(e)
         {
         }

         Data *visit(Visitor *visitor, Data *data) {
            return visitor->visitHashExpr(this, data);
         }

         Expr *e;
      };

      class IdentExpr : public Expr {
         public:
         IdentExpr(const ustring_t& s)
         : id(s)
         {
         }

         Data *visit(Visitor *visitor, Data *data) {
            return visitor->visitIdentExpr(this, data);
         }

         ustring_t id;
      };

      class IfExpr : public Expr {
         public:
         IfExpr(Expr *condition,
            Expr *then,
            const std::list<Expr*>& elsifs,
            Expr *else_)
         : condition(condition),
         then(then),
         elsifs(elsifs),
         else_(else_)
         {
         }

         Data *visit(Visitor *visitor, Data *data) {
            return visitor->visitIfExpr(this, data);
         }

         Expr *condition;
         Expr *then;
         std::list<Expr*> elsifs;
         Expr *else_;
      };

      class IntegerExpr : public Expr {
         public:
         IntegerExpr(const mpz_class& value);

         Data *visit(Visitor *visitor, Data *data) {
            return visitor->visitIntegerExpr(this, data);
         }

         mpz_class m_value;
      };

      class OpExpr : public Expr {
         public:
         OpExpr(const std::vector<Expr*>& ops, ustring_t& name)
         : m_ops(ops), m_name(name)
         {}

         Data *visit(Visitor *visitor, Data *data) {
            return visitor->visitOpExpr(this, data);
         }

         std::vector<Expr*> m_ops;
         ustring_t m_name;
      };

      class PairExpr : public Expr {
         public:
         PairExpr(Expr *lhs, Expr *rhs)
         : lhs(lhs), rhs(rhs)
         {
         }

         Data *visit(Visitor *v, Data *data) {
            return v->visitPairExpr(this, data);
         }

         Expr *lhs;
         Expr *rhs;
      };

      class RangeExpr : public Expr {
         public:
         RangeExpr(Expr *lhs, Expr *rhs)
         : lhs(lhs), rhs(rhs)
         {}

         Data *visit(Visitor *v, Data *data) {
            return v->visitRangeExpr(this, data);
         }

         Expr *lhs;
         Expr *rhs;
      };

      class SpecialOpsExpr : public Expr {
         public:
         SpecialOpsExpr(const ustring_t& op, const ustring_t& value, Expr *e)
         : value(value), e(e)
         {
            if (op == "isspecial") {
               m_f = &Visitor::visitIsSpecialExpr;
            } else if (op == "istype") {
               m_f = &Visitor::visitIsTypeExpr;
            } else if (op == "convert") {
               m_f = &Visitor::visitConvertExpr;
            }
         }

         Data *visit(Visitor *v, Data *data) {
            return m_f(v, this, data);
         }

         ustring_t value;
         Expr *e;

         private:
         boost::function<Data* (Visitor*, SpecialOpsExpr*, Data*)> m_f;
      };

      class UnaryExpr : public Expr {
         public:
         UnaryExpr(const Parser::UnaryOperation& op, Expr *e)
         : op(op), e(e)
         {
         }

         Data *visit(Visitor *v, Data *d) {
            return v->visitUnaryExpr(this, d);
         }

         Parser::UnaryOperation op;
         Expr *e;
      };

   }
}

#endif // EXPR_HPP_INCLUDED
