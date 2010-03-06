#include <tl/expr_compiler.hpp>
#include <tl/compiled_functors.hpp>
#include <boost/variant.hpp>

namespace TransLucid
{

namespace
{

struct Compiled : public AST::Data
{
  Compiled(HD* e)
  : e(e)
  {}

  HD* e;
};

}

ExprCompiler::ExprCompiler(HD* i)
: m_i(i)
{
}

ExprCompiler::~ExprCompiler()
{
}

HD*
ExprCompiler::compile(AST::Expr* e)
{
  if (e == 0)
  {
    return 0;
  }
  Compiled* c = dynamic_cast<Compiled*>(e->visit(this, 0));
  HD* h = c->e;
  delete c;
  return h;
}

AST::Data*
ExprCompiler::visitAtExpr(AST::AtExpr* e, AST::Data*)
{
  Compiled* d2 = dynamic_cast<Compiled*>(e->e2->visit(this, 0));
  Compiled* d1 = dynamic_cast<Compiled*>(e->e1->visit(this, 0));

  HD* e1 = d1->e;
  HD* e2 = d2->e;

  delete d1;
  delete d2;

  return new Compiled(e->relative ?
    static_cast<HD*>(new CompiledFunctors::AtRelative(e2, e1)) :
    static_cast<HD*>(new CompiledFunctors::AtAbsolute(e2, e1)));
}

AST::Data* ExprCompiler::visitBinaryOpExpr(AST::BinaryOpExpr* e, AST::Data*)
{
  std::vector<HD*> compiled;
  BOOST_FOREACH(AST::Expr* expr, e->operands)
  {
    Compiled* c = dynamic_cast<Compiled*>(expr->visit(this, 0));
    compiled.push_back(c->e);
    delete c;
  }

  return new Compiled(new CompiledFunctors::BinaryOp(m_i, compiled, e->op.op));
}

AST::Data*
ExprCompiler::visitBooleanExpr(AST::BooleanExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::BoolConst(e->value));
}

AST::Data*
ExprCompiler::visitBuildTupleExpr(AST::BuildTupleExpr* e, AST::Data*)
{
  std::list<std::pair<HD*, HD*>> pairs;

  BOOST_FOREACH(auto& pe, e->values)
  {
    Compiled* cl = dynamic_cast<Compiled*>(pe.first->visit(this, 0));
    Compiled* cr = dynamic_cast<Compiled*>(pe.second->visit(this, 0));
    pairs.push_back(std::make_pair(cl->e, cr->e));
    delete cl;
    delete cr;
  }

  return new Compiled(new CompiledFunctors::BuildTuple(m_i, pairs));
}

AST::Data*
ExprCompiler::visitConstantExpr(AST::ConstantExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::Constant(m_i, e->name, e->value));
}

AST::Data*
ExprCompiler::visitConvertExpr(AST::SpecialOpsExpr* e, AST::Data*)
{
  Compiled* c = dynamic_cast<Compiled*>(e->e->visit(this, 0));

  Compiled* result =
    new Compiled(new CompiledFunctors::Convert(e->value, c->e));
  delete c;
  return result;
}

AST::Data*
ExprCompiler::visitDimensionExpr(AST::DimensionExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::Dimension(m_i, e->value));
}

AST::Data*
ExprCompiler::visitHashExpr(AST::HashExpr* e, AST::Data*)
{
  Compiled* c = dynamic_cast<Compiled*>(e->e->visit(this, 0));
  HD* se = c->e;
  delete c;
  return new Compiled(new CompiledFunctors::Hash(m_i, se));
}

AST::Data*
 ExprCompiler::visitIdentExpr(AST::IdentExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::Ident(m_i, e->id));
}

AST::Data*
ExprCompiler::visitIfExpr(AST::IfExpr* e, AST::Data*)
{
  Compiled* condc = 0;
  Compiled* thenc = 0;
  Compiled* elsec = 0;

  HD* cond = 0;
  HD* then = 0;
  HD* else_ = 0;
  std::list<HD*> elseifs;

  condc = dynamic_cast<Compiled*>(e->condition->visit(this, 0));
  thenc = dynamic_cast<Compiled*>(e->then->visit(this, 0));

  BOOST_FOREACH(auto& elseif, e->elsifs)
  {
    Compiled* eicl = dynamic_cast<Compiled*>(elseif.first->visit(this, 0));
    Compiled* eicr = dynamic_cast<Compiled*>(elseif.second->visit(this, 0));
    elseifs.push_back(eicl->e);
    elseifs.push_back(eicr->e);
    delete eicl;
    delete eicr;
  }

  elsec = dynamic_cast<Compiled*>(e->else_->visit(this, 0));

  cond = condc->e;
  then = thenc->e;
  else_ = elsec->e;

  delete condc;
  delete thenc;
  delete elsec;

  return
    new Compiled(new CompiledFunctors::If(cond, then, elseifs, else_));
}

AST::Data*
ExprCompiler::visitIntegerExpr(AST::IntegerExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::Integer(m_i, e->m_value));
}

AST::Data*
ExprCompiler::visitIsSpecialExpr(AST::SpecialOpsExpr* e, AST::Data*)
{
  Compiled* c = dynamic_cast<Compiled*>(e->e->visit(this, 0));
  HD* eval = c->e;
  delete c;
  return new Compiled(new CompiledFunctors::IsSpecial(e->value, eval));
}

AST::Data*
ExprCompiler::visitIsTypeExpr(AST::SpecialOpsExpr* e, AST::Data*)
{
  Compiled* c = dynamic_cast<Compiled*>(e->e->visit(this, 0));
  HD* eval = c->e;
  delete c;
  return new Compiled(new CompiledFunctors::IsType(e->value, eval));
}

AST::Data*
ExprCompiler::visitOpExpr(AST::OpExpr* e, AST::Data*)
{
  std::vector<HD*> operands;
  BOOST_FOREACH(AST::Expr* o, e->m_ops)
  {
    Compiled* c = dynamic_cast<Compiled*>(o->visit(this, 0));
    operands.push_back(c->e);
    delete c;
  }
  return
    new Compiled(new CompiledFunctors::Operation(m_i, operands, e->m_name));
}

AST::Data*
ExprCompiler::visitPairExpr(AST::PairExpr* e, AST::Data*)
{
  Compiled* lhsc = dynamic_cast<Compiled*>(e->lhs->visit(this, 0));
  Compiled* rhsc = dynamic_cast<Compiled*>(e->rhs->visit(this, 0));

  HD* lhs = lhsc->e;
  HD* rhs = rhsc->e;

  delete lhsc;
  delete rhsc;

  return new Compiled(new CompiledFunctors::Pair(lhs, rhs));
}

AST::Data*
ExprCompiler::visitRangeExpr(AST::RangeExpr*, AST::Data*)
{
  #warning come up with a more general sets will work in guards thing and actually do the range
  return 0;
}

AST::Data*
ExprCompiler::visitSpecialExpr(AST::SpecialExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::SpecialConst(e->value));
}

AST::Data*
ExprCompiler::visitStringExpr(AST::StringExpr* e, AST::Data*)
{
  return new Compiled(new CompiledFunctors::StringConst(e->value));
}

AST::Data*
ExprCompiler::visitUcharExpr(AST::UcharExpr* e, AST::Data*) {
  return new Compiled(new CompiledFunctors::UcharConst(e->value));
}

AST::Data*
ExprCompiler::visitUnaryExpr(AST::UnaryExpr* e, AST::Data*)
{
  #if 0
  Compiled* operandc = dynamic_cast<Compiled*>(e->e->visit(this, 0));
  HD* operand = operandc->e;

  delete operandc;
  #endif

  //return new Compiled(new CompiledFunctors::UnaryOp(e->op, operand));
  return 0;
}

HD*
ExprCompiler::operator()(const Tree::AtExpr& e)
{
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);

  if (e.relative)
  {
    return new CompiledFunctors::AtRelative(lhs, rhs);
  }
  else
  {
    return new CompiledFunctors::AtAbsolute(lhs, rhs);
  }
}

HD*
ExprCompiler::operator()(const Tree::BinaryOpExpr& e)
{
  HD* lhs = boost::apply_visitor(*this, e.lhs);
  HD* rhs = boost::apply_visitor(*this, e.rhs);

  return new CompiledFunctors::BinaryOp(m_i, {lhs, rhs}, e.op.op);
}

HD*
ExprCompiler::operator()(bool b)
{
  return new CompiledFunctors::BoolConst(b);
}

HD*
ExprCompiler::operator()(const Tree::BuildTupleExpr& e)
{
  std::list<std::pair<HD*, HD*>> elements;
  BOOST_FOREACH(auto& v, e.pairs)
  {
    HD *lhs = boost::apply_visitor(*this, v.first);
    HD *rhs = boost::apply_visitor(*this, v.second);
    elements.push_back(std::make_pair(lhs, rhs));
  }
  return new CompiledFunctors::BuildTuple(m_i, elements);
}

HD*
ExprCompiler::operator()(const Tree::ConstantExpr& e)
{
  return new CompiledFunctors::Constant(m_i, e.type, e.text);
}

HD* ExprCompiler::operator()(const Tree::DimensionExpr& e)
{
  return new CompiledFunctors::Dimension(m_i, e.text);
}

HD* ExprCompiler::operator()(const Tree::HashExpr& e)
{
  HD* expr = boost::apply_visitor(*this, e.e);
  return new CompiledFunctors::Hash(m_i, expr);
}

HD*
ExprCompiler::operator()(const Tree::IdentExpr& e)
{
  return new CompiledFunctors::Ident(m_i, e.text);
}

HD*
ExprCompiler::operator()(const Tree::IfExpr& e)
{
  HD* condition = boost::apply_visitor(*this, e.condition);
  HD* then = boost::apply_visitor(*this, e.then);
  HD* else_ = boost::apply_visitor(*this, e.else_);

  std::vector<std::pair<HD*, HD*>> else_ifs;

  BOOST_FOREACH(auto& v, e.else_ifs)
  {
    else_ifs.push_back(std::make_pair
    (
      boost::apply_visitor(*this, v.first),
      boost::apply_visitor(*this, v.second)
    ));
  }

  return new CompiledFunctors::If(condition, then, else_ifs, else_);
}

HD* ExprCompiler::operator()(const mpz_class& i)
{
  return new CompiledFunctors::Integer(m_i, i);
}

HD* ExprCompiler::operator()(const u32string& s)
{
  return new CompiledFunctors::StringConst(s);
}

HD* ExprCompiler::operator()(const Tree::UnaryOpExpr& e)
{
  HD* operand = boost::apply_visitor(*this, e.e);
  return new CompiledFunctors::UnaryOp(e.op, operand);
}

} //namespace TransLucid
