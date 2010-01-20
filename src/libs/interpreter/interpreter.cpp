#include <tl/interpreter.hpp>
#include <tl/parser.hpp>
#include <glibmm/convert.h>
#include <glibmm/fileutils.h>
#include <glibmm/miscutils.h>
#include <tl/range.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>
#include <tl/constant.hpp>
#include <tl/consthd.hpp>

namespace TransLucid {

inline void Interpreter::addToVariableActual(const ustring_t& id,
   const Tuple& k, HD *h)
{
   //std::cerr << "addToVariableActual: " <<
   //   id << std::endl;
   //find the variable
   VariableMap::const_iterator iter = m_variables.find(id);
   if (iter == m_variables.end()) {
      //std::cerr << "constructing new variable" << std::endl;
      iter = m_variables.insert(std::make_pair(id, new Variable(id, *this))).first;
   }
   iter->second->addExpr(k, h);
}

inline void Interpreter::addToVariable(const ustring_t& id,
   const ustring_t& remaining, const Tuple& k, HD *h)
{
   tuple_t kp = k.tuple();
   kp[m_dimension_id] = TypedValue(String(remaining),
      m_types.indexString());
   addToVariableActual(id, Tuple(kp), h);
}

inline void Interpreter::addToVariable(const ustring_t& id,
   const Tuple& k, HD *h)
{
   tuple_t kp = k.tuple();
   kp.erase(m_dimension_id);
   addToVariableActual(id, Tuple(kp), h);
}

template <typename T>
void Interpreter::buildConstantHD() {
   HD *h = new T(*this);

   tuple_t k;
   //k[m_dimTranslator.lookup("id")] = TypedValue(String("CONST"), m_typeRegistry.indexString());
   k[m_dimTranslator.lookup("type")] = TypedValue(String(T::name), m_types.indexString());
   addToVariableActual("CONST", Tuple(k), h);
}

Interpreter::Interpreter()
: m_types(*this), m_evaluator(*this),
m_maxClock(0),
m_warehouse(*this),
m_dimension_id(m_dimTranslator.lookup("id")),
m_verbose(false)
{
   //m_systemParser.push(*m_systemGrammar);
   initExprParser();
   initTupleParser();
   initConstantParser();
   m_dimTranslator.lookup("time");
   m_dimTranslator.lookup("priority");
   m_dimTranslator.lookup("_validguard");
   Parser::addSymbol(L"time", m_parseInfo.dimension_names, m_parseInfo.dimension_symbols);

   //create the obj, const and fun ids
   //m_variables.insert(std::make_pair("const", new ConstantHD(*this)));

   //build the constant creators
   buildConstantHD<ConstHD::UChar>();
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

void Interpreter::addExpr(const Tuple& k, HD *h) {
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
      //set the equation for the first component
      addToVariable(split.first(), split.last(), k, h);
   } else {
      //set the equation for the whole name
      addToVariable(id->value(), k, h);
   }
}

TaggedValue Interpreter::operator()(const Tuple& k) {
   //the interpreter understands requests for an id
   //therefore, look for an id and evaluate it
   Tuple::const_iterator iditer = k.find(m_dimension_id);

   if (iditer == k.end()) {
      //std::cerr << "could not find id dimension " << std::endl;
      return TaggedValue(TypedValue(Special(Special::DIMENSION), m_types.indexSpecial()), k);
   }

   try {
      VariableMap::const_iterator viter = m_variables.find(iditer->second.value<String>().value());
      if (viter == m_variables.end()) {
         return TaggedValue(TypedValue(Special(Special::UNDEF), m_types.indexSpecial()), k);
      } else {
         return (*viter->second)(k);
      }
   } catch (std::bad_cast& e) {
      return TaggedValue(TypedValue(Special(Special::DIMENSION), m_types.indexSpecial()), k);
   }
}

} //namespace TransLucid
