#include "tlcore.hpp"
#include <tl/expr_parser.hpp>

namespace TransLucid
{

namespace TLCore
{

template <typename Iterator>
class Grammar : 
  public Parser::qi::grammar<Iterator, Parser::SkipGrammar<Iterator>>
{
  public:
  Grammar()
  : m_expr(m_header)
  {
  }

  private:
  Parser::Header m_header;
  Parser::ExprGrammar<Iterator> m_expr;
};

TLCore::TLCore()
: m_verbose(false)
{
}

void TLCore::run()
{
}

} //namespace TLCore

} //namespace TransLucid
