#include <tl/interpreter.hpp>
#include <tl/parser.hpp>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <tl/range.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>

namespace TransLucid {

Interpreter::Interpreter()
: m_types(*this), m_evaluator(*this),
m_maxClock(0),
m_warehouse(*this),
m_dimension_id(m_dimTranslator.insert("id")),
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

void Interpreter::addExpr(const Tuple& k, AST::Expr *e) {
   Tuple::const_iterator iter = k.find(m_dimension_id);
   if (iter == k.end()) {
      return;
   }

   const String *id = iter->second.valuep<String>();
   if (id == 0) {
      return;
   }

   SplitID split(id->value());

   if (split.has_components()) {
      //and the end to the first
   } else {
      //set the equation for the whole name
   }
}

TaggedValue Interpreter::operator()(const Tuple& k) {
}

} //namespace TransLucid
