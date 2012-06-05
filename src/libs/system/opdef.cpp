#include <tl/opdef.hpp>

namespace TransLucid
{

Constant
OpDefWS::operator()(Context& k)
{
  return m_bestfit(k);
}

Constant
OpDefWS::operator()(Context& kappa, Context& delta)
{
  return m_bestfit(kappa, delta);
}

bool 
OpDefWS::del(uuid id, size_t time)
{
  return m_bestfit.del(id, time);
}

bool 
OpDefWS::repl(uuid id, size_t time, Parser::Line line)
{
  return m_bestfit.repl(id, time, line);
}

void
OpDefWS::addEquation(uuid id, Parser::RawInput input, int time)
{
  m_bestfit.addEquation(id, input, time);
}

Tree::Expr
OpDefWS::group(const std::list<EquationDefinition>& defs)
{
}

}
