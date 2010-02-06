#if 0

#include <iostream>
#include <tl/types.hpp>
#include <tl/expr.hpp>
#include <tl/interpreter.hpp>
#include <tl/builtin_types.hpp>

namespace TL = TransLucid;

namespace {

using namespace TL::AST;

class Printer : public TL::TypedValueBase {
   public:

   enum Type {
      ERROR,
      MAXIMAL,
      MINIMAL,
      GRAPH
   };

   Printer(Type t)
   : m_type(t)
   {
   }

   size_t hash() const {
      return m_type;
   }

   void print(std::ostream& os, const TL::Tuple& c, TL::Interpreter& i) const;

   bool operator==(const Printer& rhs) const {
      return m_type == rhs.m_type;
   }

   bool operator<(const Printer& rhs) const {
      return m_type < rhs.m_type;
   }

   private:
   Type m_type;
};

class PrintVisitor : public TL::AST::Visitor {
   public:
   PrintVisitor(std::ostream& os, Printer::Type type)
   : m_os(os), m_type(type)
   {
   }

   void evaluate(TL::AST::Expr* e) {
      e->visit(this, 0);
   }

   Data * visitAtExpr(AtExpr*, Data*);
   Data * visitBinaryOpExpr(BinaryOpExpr*, Data*);
   Data * visitBooleanExpr(BooleanExpr*, Data*);
   Data * visitBuildTupleExpr(BuildTupleExpr*, Data*);
   Data * visitConstantExpr(ConstantExpr*, Data*);
   Data * visitConvertExpr(SpecialOpsExpr*, Data*);
   Data * visitDimensionExpr(DimensionExpr*, Data*);
   Data * visitHashExpr(HashExpr*, Data*);
   Data * visitIdentExpr(IdentExpr*, Data*);
   Data * visitIfExpr(IfExpr*, Data*);
   Data * visitIntegerExpr(IntegerExpr*, Data*);
   Data * visitIsSpecialExpr(SpecialOpsExpr*, Data*);
   Data * visitIsTypeExpr(SpecialOpsExpr*, Data*);
   Data * visitOpExpr(OpExpr*, Data*);
   Data * visitPairExpr(PairExpr*, Data*);
   Data * visitRangeExpr(RangeExpr*, Data*);
   Data * visitUnaryExpr(UnaryExpr*, Data*);

   private:
   std::ostream& m_os;
   Printer::Type m_type;
};

class PrinterManager : public TL::TypeManager {
   public:
   PrinterManager(TL::TypeRegistry& r)
   : TypeManager(r, "printer"),
   m_interpreter(r.interpreter())
   {
   }

   TL::TypedValue parse(const TL::ustring_t& text, const TL::Tuple& c, TL::Interpreter& interpreter) const {
      Printer::Type type;
      if (text == "graph") {
         type = Printer::GRAPH;
      } else if (text == "minimal") {
         type = Printer::MINIMAL;
      } else if (text == "maximal") {
         type = Printer::MAXIMAL;
      } else {
         type = Printer::ERROR;
      }

      return TL::TypedValue(Printer(type), index());
   }

   void printInternal(std::ostream& os, const TL::TypedValue& v, const TL::Tuple& c) const {
      const Printer& p = v.value<Printer>();
      p.print(os, c, m_interpreter);
   };

   private:
   TL::Interpreter& m_interpreter;
};

void Printer::print(std::ostream& os, const TL::Tuple& c, TL::Interpreter& i) const {
   #if 0
   if (m_type == ERROR) {
      os << "printer<error>";
   } else {
      size_t dim = i.dimTranslator().lookup("printeqn");

      TL::Tuple::const_iterator iter;
      if (dim != 0 && (iter = c.find(dim)) != c.end()) {
         const TL::TypedValue& v = iter->second;
         if (v.index() == i.typeRegistry().indexExpr()) {
            const TL::ExprType& e = v.value<TL::ExprType>();
            os << "printer<";
            PrintVisitor p(os, m_type);
            p.evaluate(e.expr());
            os << ">";
         } else {
            os << "special<typeerror>";
         }
      } else {
         os << "special<dim>";
      }
   }
   #endif
}

Data * PrintVisitor::visitBinaryOpExpr(BinaryOpExpr* e, Data*) {
   #warning fix the printer
   #if 0
   m_os << "(";
   switch (e->op.assoc) {
      case TL::Parser::ASSOC_LEFT:
      case TL::Parser::ASSOC_RIGHT:
      case TL::Parser::ASSOC_NON:
      e->operands.at(0)->visit(this, 0);
      m_os << " " << e->op.symbol << " ";
      e->operands.at(1)->visit(this, 0);
      break;

      case TL::Parser::ASSOC_COMPARISON:
      case TL::Parser::ASSOC_VARIABLE:
      {
         std::vector<Expr*>::const_iterator rhs = e->operands.begin();
         std::vector<Expr*>::const_iterator lhs = rhs++;

         (*lhs)->visit(this, 0);

         while (rhs != e->operands.end()) {
            m_os << " " << e->op.symbol << " ";
            (*rhs)->visit(this, 0);
            ++rhs;
            ++lhs;
         }
      }
      break;
   }
   m_os << ")";

   #endif

   return 0;
}

/** @brief visitIsTypeExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitIsTypeExpr(SpecialOpsExpr* e, Data*)
{
   if (m_type == Printer::MAXIMAL) {
      m_os << '(';
   }

   m_os << "istype<" << e->value << ">";
   e->e->visit(this, 0);

   if (m_type == Printer::MAXIMAL) {
      m_os << ')';
   }

   return 0;
}

Data * PrintVisitor::visitBuildTupleExpr(BuildTupleExpr* e, Data*)
{
   m_os << '[';

   bool first = true;
   BOOST_FOREACH(Expr *p, e->values) {
      if (first) {
         first = false;
      } else {
         m_os << ", ";
      }
      p->visit(this, 0);
   }

   m_os << ']';

   return 0;
}

/** @brief visitPairExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitPairExpr(PairExpr* e, Data*)
{
   e->lhs->visit(this, 0);
   m_os << " : ";
   e->rhs->visit(this, 0);
   return 0;
}

/** @brief visitIdentExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitIdentExpr(IdentExpr* e, Data*)
{
   m_os << "ident<" << e->id << ">";
   return 0;
}

/** @brief visitIfExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitIfExpr(IfExpr* e, Data*)
{
   m_os << "if ";
   e->condition->visit(this, 0);
   m_os << " then ";
   e->then->visit(this, 0);

   std::list<Expr*>::const_iterator iter = e->elsifs.begin();
   while (iter != e->elsifs.end()) {
      m_os << " elsif ";
      (*iter)->visit(this, 0);
      m_os << " then ";
      ++iter;
      (*iter)->visit(this, 0);
      ++iter;
   }
   m_os << " else ";
   e->else_->visit(this, 0);
   m_os << " fi";
   return 0;
}

/** @brief visitIntegerExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitIntegerExpr(IntegerExpr* e, Data*)
{
   m_os << e->m_value;
   return 0;
}

/** @brief visitIsSpecialExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitIsSpecialExpr(SpecialOpsExpr* e, Data*)
{
   if (m_type == Printer::MAXIMAL) {
      m_os << '(';
   }

   m_os << "isspecial<" << e->value << "> ";
   e->e->visit(this, 0);

   if (m_type == Printer::MAXIMAL) {
      m_os << ')';
   }
   return 0;
}

/** @brief visitDimensionExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitDimensionExpr(DimensionExpr* e, Data*)
{
   m_os << "dim<" << e->value << ">";
   return 0;
}

/** @brief visitHashExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitHashExpr(HashExpr* e, Data*)
{
   if (m_type == Printer::MAXIMAL) {
      m_os << "(";
   }

   m_os << "# ";
   e->e->visit(this, 0);

   if (m_type == Printer::MAXIMAL) {
      m_os << ")";
   }

   return 0;
}

/** @brief visitConstantExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitConstantExpr(ConstantExpr* e, Data*)
{
   m_os << e->name << "<" << e->value << ">";
   return 0;
}

/** @brief visitBooleanExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitBooleanExpr(BooleanExpr* e, Data*)
{
   if (e->value) {
      m_os << "true";
   } else {
      m_os << "false";
   }
   return 0;
}

/** @brief visitAtExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitAtExpr(AtExpr* e, Data*)
{
   if (m_type == Printer::MAXIMAL) {
      m_os << "(";
   }

   e->e2->visit(this, 0);
   m_os << " @ ";
   e->e1->visit(this, 0);

   if (m_type == Printer::MAXIMAL) {
      m_os << ")";
   }

   return 0;
}

/** @brief visitConvertExpr
  *
  * @todo: document this function
  */
Data * PrintVisitor::visitConvertExpr(SpecialOpsExpr* e, Data*)
{
   if (m_type == Printer::MAXIMAL) {
      m_os << '(';
   }

   m_os << "convert<" << e->value << "> ";
   e->e->visit(this, 0);

   if (m_type == Printer::MAXIMAL) {
      m_os << ')';
   }
   return 0;
}

Data * PrintVisitor::visitRangeExpr(RangeExpr *e, Data *) {
   if (e->lhs) {
      e->lhs->visit(this, 0);
   } else {
      m_os << "inf";
   }

   m_os << " .. ";

   if (e->rhs) {
      e->rhs->visit(this, 0);
   } else {
      m_os << "inf";
   }
   return 0;
}

Data * PrintVisitor::visitUnaryExpr(UnaryExpr *e, Data*) {
   return 0;
}

}

extern "C" {
   void library_init(TransLucid::TypeRegistry& r) {
      new PrinterManager(r);
   }
}

#endif
