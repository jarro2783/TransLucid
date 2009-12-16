#include <tl/interpreter.hpp>
#include <tl/parser.hpp>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <tl/range.hpp>
#include <tl/builtin_types.hpp>

namespace TransLucid {

Interpreter::Interpreter()
: m_types(*this), m_evaluator(*this),
m_maxClock(0),
m_warehouse(*this),
m_verbose(false)
{
   //m_systemParser.push(*m_systemGrammar);
   initExprParser();
   initTupleParser();
   initConstantParser();
   m_dimTranslator.insert("time");
   m_dimTranslator.insert("priority");
   Parser::addSymbol(L"time", m_parseInfo.dimension_names, m_parseInfo.dimension_symbols);
}

Interpreter::~Interpreter() {
   cleanupExprParser();
   cleanupTupleParser();
   cleanupConstantParser();
}

bool Interpreter::parseString(
   const ustring_t& s,
   const Parser::Spirit::stored_rule<Parser::scanner_t>& parser,
   const ustring_t& name)
{
   Parser::UIterator iter(s);
   Parser::UIterator end = iter.make_end();
   return parseRange(
      Parser::iterator_t(
         Parser::Iterator(iter),
         Parser::Iterator(end)
         ),
      Parser::iterator_t(), parser, name);
}

AST::Expr *Interpreter::parseExpr(const ustring_t& s) {
   AST::Expr *e = 0;
   if (parseString(s, m_parsers.expr_parser.top())) {
      e = m_parsers.expr_stack.at(0);
      m_parsers.expr_stack.pop_front();
   }

   return e;
}

#if 0
bool Interpreter::parseRange(
   Parser::iterator_t begin,
   Parser::iterator_t end,
   const Parser::Spirit::stored_rule<Parser::scanner_t>& parser,
   const ustring_t& name)
{

}
#endif

void Interpreter::cleanupParserObjects() {
   #if 0
   BOOST_FOREACH(Parser::EqnMap::value_type& v, m_parseInfo.equations) {
      BOOST_FOREACH(Parser::EqnMap::mapped_type::value_type& p, v.second) {
         delete p.first;
         delete p.second;
      }
   }
   #endif

   //m_parseInfo.equations.clear();

   BOOST_FOREACH(AST::Expr *e, m_parsers.expr_stack) {
      delete e;
   }
   m_parsers.expr_stack.clear();
}

EquationSetIterator Interpreter::equationsBegin() {
   return EquationSetIterator(eqnSets.begin(), m_variables);
}

EquationSetIterator Interpreter::equationsEnd() {
   return EquationSetIterator(eqnSets.end(), m_variables);
}

EquationSet Interpreter::createEquationSet(const EquationGuard& guard) {
   boost::shared_ptr<EquationMap> p(new EquationMap);
   EqnSetList::const_iterator inserted =
      eqnSets.insert(eqnSets.end(), std::make_pair(guard, p));
   return EquationSet(inserted, m_variables);
}

Tuple EquationGuard::evaluate(Interpreter& i, const Tuple& context) const
   throw (InvalidGuard)
{

   tuple_t t = m_dimensions;

   if (m_guard) {
      ValueContext v = i.evaluate(m_guard, context);

      if (v.first.index() == i.typeRegistry().indexTuple()) {
         BOOST_FOREACH(const Tuple::value_type& value, v.first.value<Tuple>()) {
            if (t.find(value.first) != t.end()) {
               throw InvalidGuard();
            }

            t.insert(std::make_pair(value.first, value.second));
         }
      } else {
         throw ParseError(__FILE__ ":" STRING_(__LINE__) ": guard is not a tuple");
      }
   }

   return Tuple(t);
}

} //namespace TransLucid
