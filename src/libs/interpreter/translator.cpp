#include <tl/translator.hpp>
#include <tl/parser.hpp>
#include <tl/expr_parser.hpp>
#include <tl/tuple_parser.hpp>
#include <boost/spirit/home/qi/parser.hpp>

namespace TransLucid
{

namespace
{
  namespace qi = boost::spirit::qi;

  template <typename Iterator>
  class EquationSetGrammar
  : public qi::grammar<Iterator, std::vector<Parser::ParsedEquation>()>
  {
    public:
    template <typename T>
    EquationSetGrammar(T& t)
    : EquationSetGrammar::base_type(equations)
    {
      using boost::phoenix::push_back;
      using namespace qi::labels;

      one_equation = t
        [
          _val = _1
        ]
      ;

      equations = *(one_equation >> ";;")
        [
          push_back(_val, _1)
        ]
      ;
    }

    private:
    qi::rule<Iterator, std::vector<Parser::ParsedEquation>()> equations;
    qi::rule<Iterator, Parser::ParsedEquation()> one_equation;
  };
}

Translator::Translator()
: m_compiler(&m_interpreter)
{
  m_header = new Parser::Header;

  m_expr = new Parser::ExprGrammar<Parser::iterator_t>(*m_header);
  m_equation = new Parser::EquationGrammar<Parser::iterator_t>;
  m_tuple = new Parser::TupleGrammar<Parser::iterator_t>;
  m_skipper = new Parser::SkipGrammar<Parser::iterator_t>;

  m_expr->set_context_perturb(*m_tuple);
  m_tuple->set_expr(*m_expr);

  m_header->delimiter_start_symbols.add(L"«",
    Parser::Delimiter(U"ustring", L'«', L'»'));
  m_header->delimiter_start_symbols.add(L"\'",
    Parser::Delimiter(U"uchar", '\'', '\''));
}

HD* Translator::translate_expr(const Parser::string_type& s)
{
  Parser::iterator_t pos = s.begin();
  AST::Expr* e = 0;

  boost::spirit::qi::phrase_parse(
    pos,
    s.cend(),
    *m_expr,
    *m_skipper,
    e);

  return m_compiler.compile(e);
}

void Translator::translate_equation_set(const u32string& s)
{
  Parser::string_type ws(s.begin(), s.end());
  Parser::iterator_t pos = ws.begin();

  EquationSetGrammar<Parser::iterator_t> equation_set(*m_equation);
  std::vector<Parser::ParsedEquation> parsedEquations;

  boost::spirit::qi::phrase_parse(
    pos,
    ws.cend(),
    equation_set,
    *m_skipper,
    parsedEquations);
}

}
