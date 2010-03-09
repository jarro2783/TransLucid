#include <tl/tree_printer.hpp>
#include <tl/translator.hpp>
#include <tl/parser.hpp>
#include <tl/expr_parser.hpp>
#include <tl/tuple_parser.hpp>
#include <boost/spirit/home/qi/parser.hpp>
#include <tl/types.hpp>
#include <tl/utility.hpp>

namespace std
{
  ostream& operator<<
  (
    std::ostream& os,
    const TransLucid::Parser::ParsedEquation& e
  )
  {
    #if 0
    os
    << "<"
    << TransLucid::utf32_to_utf8(TransLucid::to_u32string(std::get<0>(e)))
    << ", " << std::get<1>(e) << ", " <<
      std::get<2>(e) << ", " << std::get<3>(e) << ">";
    #endif
    return os;
  }
}

namespace TransLucid
{

namespace
{
  namespace qi = boost::spirit::qi;

  template <typename Iterator>
  class EquationSetGrammar
  : public qi::grammar<Iterator, std::vector<Parser::ParsedEquation>(),
    Parser::SkipGrammar<Iterator>>
  {
    public:
    template <typename T>
    EquationSetGrammar(T& t)
    : EquationSetGrammar::base_type(equations)
    {
      using boost::phoenix::push_back;
      using namespace qi::labels;

      eqn %= t;

      one_equation = eqn
        [
          _val = _1
        ]
      ;

      equations %= *(one_equation >> ";;") >> qi::eoi
        //[
        //  push_back(_val, _1)
        //]
      ;

      BOOST_SPIRIT_DEBUG_NODE(one_equation);
      //BOOST_SPIRIT_DEBUG_NODE(equations);
    }

    private:

    qi::rule
    <
      Iterator,
      std::vector<Parser::ParsedEquation>(),
      Parser::SkipGrammar<Iterator>
    > equations;

    qi::rule
    <
      Iterator,
      Parser::ParsedEquation(),
      Parser::SkipGrammar<Iterator>
    >
      one_equation,
      eqn
    ;
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

  m_equation->set_expr(*m_expr);

  m_header->delimiter_start_symbols.add(L"«",
    Parser::Delimiter(U"ustring", L'«', L'»'));
  m_header->delimiter_start_symbols.add(L"\'",
    Parser::Delimiter(U"uchar", '\'', '\''));
}

HD* Translator::translate_expr(const Parser::string_type& s)
{
  Parser::iterator_t pos = s.begin();
  Tree::Expr e;

  boost::spirit::qi::phrase_parse(
    pos,
    s.cend(),
    *m_expr,
    *m_skipper,
    e);

  m_lastExpr = e;

  return m_compiler.compile(e);
}

equation_v
Translator::translate_equation_set(const u32string& s)
{
  Parser::string_type ws(s.begin(), s.end());
  Parser::iterator_t pos = ws.begin();

  EquationSetGrammar<Parser::iterator_t> equation_set(*m_equation);
  std::vector<Parser::ParsedEquation> parsedEquations;

  bool success = boost::spirit::qi::phrase_parse(
    pos,
    ws.cend(),
    equation_set,
    *m_skipper,
    parsedEquations);

  if (!success)
  {
    throw "failed parsing equations";
  }

  equation_v equations;

  BOOST_FOREACH(auto& v, parsedEquations)
  {
    HD* context = m_compiler.compile(std::get<1>(v));
    HD* boolean = m_compiler.compile(std::get<2>(v));
    HD* e = m_compiler.compile(std::get<3>(v));
    equations.push_back(TranslatedEquation(
      to_u32string(std::get<0>(v)),
      context,
      boolean,
      e));

    typedef std::back_insert_iterator<std::string> out_iter;
    Printer::ExprPrinter<out_iter> print_grammar;
    std::string generated;
    std::back_insert_iterator<std::string> outit(generated);

    std::cerr << "parsed equation: " << std::endl;
    std::cerr << "name: " << utf32_to_utf8(to_u32string(std::get<0>(v)))
    << std::endl;

    Printer::karma::generate(outit, print_grammar, std::get<1>(v));
    std::cerr << "guard: " << generated << std::endl;

    generated.clear();
    Printer::karma::generate(outit, print_grammar, std::get<2>(v));
    std::cerr << "boolean: " << generated << std::endl;

    generated.clear();
    Printer::karma::generate(outit, print_grammar, std::get<3>(v));
    std::cerr << "equation: " << generated << std::endl;
  }

  return equations;
}

void
Translator::translate_and_add_equation_set(const u32string& s)
{
  equation_v equations = translate_equation_set(s);

  BOOST_FOREACH(auto& v, equations)
  {
    m_interpreter.addExpr(
      Tuple(create_add_eqn_context(to_u32string(std::get<0>(v)),
                                     std::get<1>(v),
                                     std::get<2>(v))),
                          std::get<3>(v));
  }
}

}
