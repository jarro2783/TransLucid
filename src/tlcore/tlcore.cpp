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
  : Grammar::base_type(r_program)
   ,m_expr(m_header)
  {
    r_program = 
       r_header
    >> "%%"
    >> r_eqns
    >> "%%"
    >> r_exprs
    >> Parser::qi::eoi
    ;
  }

  private:
  Parser::Header m_header;
  Parser::ExprGrammar<Iterator> m_expr;

  Parser::qi::rule<Iterator, Parser::SkipGrammar<Iterator>> 
    r_program,
    r_header,
    r_exprs,
    r_eqns
  ;
};

TLCore::TLCore()
: 
  m_verbose(false)
 ,m_reactive(false)
{
  m_grammar = new Grammar<Parser::string_type::const_iterator>;
}

void TLCore::run()
{
}

} //namespace TLCore

} //namespace TransLucid
