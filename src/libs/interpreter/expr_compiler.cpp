#include <tl/expr_compiler.hpp>
#include <tl/set_lazy_evaluator.hpp>

namespace TransLucid {

namespace {

struct Compiled : public AST::Data {

   Compiled(HD *e)
   : e(e)
   {}

   HD *e;
};

}

ExprCompiler::ExprCompiler()
{
   //ctor
}

ExprCompiler::~ExprCompiler()
{
   //dtor
}

AST::Data *ExprCompiler::visitAtExpr(AST::AtExpr* e, AST::Data*) {
   Compiled *d2 = dynamic_cast<Compiled*>(e->e2->visit(this, 0));
   Compiled *d1 = dynamic_cast<Compiled*>(e->e1->visit(this, 0));

   SetEvaluator *e1 = d1->e;
   SetEvaluator *e2 = d2->e;

   delete d1;
   delete d2;

   return new Compiled(e->relative ?
      static_cast<SetEvaluator*>(new SetLazyEvaluator::AtRelative(e2, e1)) :
      static_cast<SetEvaluator*>(new SetLazyEvaluator::AtAbsolute(e2, e1)));
}

AST::Data *ExprCompiler::visitBinaryOpExpr(AST::BinaryOpExpr *e, AST::Data*) {
   std::vector<SetEvaluator*> compiled;
   BOOST_FOREACH(AST::Expr *expr, e->operands) {
      Compiled *c = dynamic_cast<Compiled*>(expr->visit(this, 0));
      compiled.push_back(c->e);
      delete c;
   }
}

AST::Data *ExprCompiler::visitBooleanExpr(AST::BooleanExpr* e, AST::Data*) {
   return new Compiled(new SetLazyEvaluator::Boolean(e->value));
}

AST::Data *ExprCompiler::visitBuildTupleExpr(AST::BuildTupleExpr* e, AST::Data*) {
   std::list<SetEvaluator*> pairs;

   BOOST_FOREACH(AST::Expr *pe, e->values) {
      Compiled *c = dynamic_cast<Compiled*>(pe->visit(this, 0));
      pairs.push_back(c->e);
      delete c;
   }

   return new Compiled(new SetLazyEvaluator::BuildTuple(pairs));
}

AST::Data *ExprCompiler::visitConstantExpr(AST::ConstantExpr* e, AST::Data*) {
   return new Compiled(new SetLazyEvaluator::Constant(e->name, e->value));
}

AST::Data *ExprCompiler::visitConvertExpr(AST::SpecialOpsExpr* e, AST::Data*) {
   Compiled *c = dynamic_cast<Compiled*>(e->e->visit(this, 0));

   Compiled *result = new Compiled(new SetLazyEvaluator::Convert(e->value, c->e));
   delete c;
   return result;
}

AST::Data *ExprCompiler::visitDimensionExpr(AST::DimensionExpr* e, AST::Data*) {
   return new Compiled(new SetLazyEvaluator::Dimension(e->value));
}

AST::Data *ExprCompiler::visitHashExpr(AST::HashExpr* e, AST::Data*) {
   Compiled *c = dynamic_cast<Compiled*>(e->e->visit(this, 0));
   SetEvaluator *se = c->e;
   delete c;
   return new Compiled(new SetLazyEvaluator::Hash(se));
}

AST::Data *ExprCompiler::visitIdentExpr(AST::IdentExpr* e, AST::Data*) {
   return new Compiled(new SetLazyEvaluator::Ident(e->id));
}

AST::Data *ExprCompiler::visitIfExpr(AST::IfExpr* e, AST::Data*) {
   Compiled *condc = 0;
   Compiled *thenc = 0;
   Compiled *elsec = 0;

   SetEvaluator *cond = 0;
   SetEvaluator *then = 0;
   SetEvaluator *else_ = 0;
   std::list<SetEvaluator*> elseifs;

   condc = dynamic_cast<Compiled*>(e->condition->visit(this, 0));
   thenc = dynamic_cast<Compiled*>(e->then->visit(this, 0));

   BOOST_FOREACH(AST::Expr *elseif, e->elsifs) {
      Compiled *eic = dynamic_cast<Compiled*>(elseif->visit(this, 0));
      elseifs.push_back(eic->e);
      delete eic;
   }

   elsec = dynamic_cast<Compiled*>(e->else_->visit(this, 0));

   cond = condc->e;
   then = thenc->e;
   else_ = elsec->e;

   delete condc;
   delete thenc;
   delete elsec;

   return new Compiled(new SetLazyEvaluator::If(cond, then, elseifs, else_));
}

AST::Data *ExprCompiler::visitIntegerExpr(AST::IntegerExpr* e, AST::Data*) {
   return new Compiled(new SetLazyEvaluator::Integer(e->m_value));
}

AST::Data *ExprCompiler::visitIsSpecialExpr(AST::SpecialOpsExpr* e, AST::Data*) {
   Compiled *c = dynamic_cast<Compiled*>(e->e->visit(this, 0));
   SetEvaluator *eval = c->e;
   delete c;
   return new Compiled(new SetLazyEvaluator::IsSpecial(e->value, eval));
}

AST::Data *ExprCompiler::visitIsTypeExpr(AST::SpecialOpsExpr* e, AST::Data*) {
   Compiled *c = dynamic_cast<Compiled*>(e->e->visit(this, 0));
   SetEvaluator *eval = c->e;
   delete c;
   return new Compiled(new SetLazyEvaluator::IsType(e->value, eval));
}

AST::Data *ExprCompiler::visitPairExpr(AST::PairExpr* e, AST::Data*) {
   Compiled *lhsc = dynamic_cast<Compiled*>(e->lhs->visit(this, 0));
   Compiled *rhsc = dynamic_cast<Compiled*>(e->rhs->visit(this, 0));

   SetEvaluator *lhs = lhsc->e;
   SetEvaluator *rhs = rhsc->e;

   delete lhsc;
   delete rhsc;

   return new Compiled(new SetLazyEvaluator::Pair(lhs, rhs));
}

AST::Data *ExprCompiler::visitRangeExpr(AST::RangeExpr*, AST::Data*) {
}

AST::Data *ExprCompiler::visitUnaryExpr(AST::UnaryExpr* e, AST::Data*) {
   Compiled *operandc = dynamic_cast<Compiled*>(e->e->visit(this, 0));
   SetEvaluator *operand = operandc->e;

   delete operandc;

   return new Compiled(new SetLazyEvaluator::UnaryOp(e->op, operand));
}

} //namespace TransLucid
