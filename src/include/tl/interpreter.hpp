#ifndef INTERPRETER_HPP_INCLUDED
#define INTERPRETER_HPP_INCLUDED

#include <tl/library.hpp>
#include <tl/types.hpp>
#include <glibmm/convert.h>
#include <tl/evaluator.hpp>
#include <tl/dimtranslator.hpp>
#include <boost/foreach.hpp>
#include <tl/parser_fwd.hpp>
#include <deque>
#include <boost/assign.hpp>
#include <tl/cache.hpp>
#include <tl/equation.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid {

   class Interpreter;

#if 0
   class SetEvaluator {
      public:
      virtual ~SetEvaluator() {
      }

      virtual TaggedValue evaluate(const TupleSet& context,Interpreter& i) = 0;
   };
#endif

   class SingleEvaluator {
   };

   /**
    * @brief Interpreter base class.
    *
    * Holds all the data necessary for an interpreter.
    **/
   class Interpreter : public HD {
      public:

      Interpreter();
      virtual ~Interpreter();

      /**
       * @brief Loads a library.
       *
       * Initialises the library which will add its types and operations
       * to the type registry.
       **/
      void loadLibrary(const ustring_t& name) {
         m_lt.loadLibrary(name, *this);
      }

      /**
       * @brief Adds a search path for loading libraries.
       *
       * When a library is loaded the paths added here will be searched
       * for the library in the order they are added.
       **/
      void addLibrarySearchPath(const ustring_t& name) {
         m_lt.addSearchPath(name);
      }

      TypeRegistry& typeRegistry() {
         return m_types;
      }

      const TypeRegistry& typeRegistry() const {
         return m_types;
      }

      DimensionTranslator& dimTranslator() {
         return m_dimTranslator;
      }

      size_t registerDimension(const ustring_t& name);

      void registerEquation(const ustring_t& name,
         const Tuple& validContext,
         AST::Expr *e);

      AST::Expr *lookupEquation(const ustring_t& name, const Tuple& c);

      //ValueContext evaluate(AST::Expr *e, const Tuple& c) {
      //   return m_evaluator.evaluate(e, c);
      //}

      AST::Expr* parseExpr(const ustring_t& s);

      int errorCount() const {
         return m_parseInfo.errorCount;
      }

      void verbose(bool v = true) {
         m_verbose = v;
      }

      Parser::Parsers& parsers() {
         return m_parsers;
      }

      template <typename P>
      bool parseRange(
         Parser::iterator_t begin,
         Parser::iterator_t end,
         const P& parser,
         const ustring_t& name = ustring_t());

      #if 0
      HD *lookupVariable(const ustring_t& name) {
         VariableMap::const_iterator iter = m_variables.find(name);
         return iter != m_variables.end() ? iter->second : 0;
      }
      #endif

      void listVariables() {
         BOOST_FOREACH(const VariableMap::value_type& v, m_variables) {
            std::cout << v.first << std::endl;
         }
      }

      LazyWarehouse& warehouse() {
         return m_warehouse;
      }

      TaggedValue operator()(const Tuple& k);

      void addExpr(const Tuple& k, HD *h);

      //int add(const Context& context,

      private:
      Libtool m_lt;
      TypeRegistry m_types;
      //Evaluator m_evaluator;
      DimensionTranslator m_dimTranslator;
      mpz_class m_maxClock;
      LazyWarehouse m_warehouse;

      //the demands for each thread
      std::vector<std::vector<std::pair<AST::Expr*, AST::Expr*> > > m_demands;

      //std::vector<std::pair<AST::Expr*, std::map<ustring_t, std::vector<std::pair<AST::Expr*, AST::Expr*> > > >
      EqnSetList eqnSets;

      //everything related to the parser
      void initExprParser();
      void cleanupExprParser();
      void initTupleParser();
      void cleanupTupleParser();
      void initConstantParser();
      void cleanupConstantParser();
      //Parser::SystemGrammar *m_systemGrammar;
      Parser::ExprGrammar *m_exprGrammar;
      Parser::TupleGrammar *m_tupleGrammar;
      Parser::ConstantGrammar *m_constantGrammar;
      VariableMap m_variables;

      //special dimensions
      size_t m_dimension_id;

      //adds to id with remaining in the id dimension
      void addToVariable(const ustring_t& id, const ustring_t& remaining,
         const Tuple& k, HD *e);
      //adds to id removing id from the context
      void addToVariable(const ustring_t& id,
         const Tuple& k, HD *e);
      //does the actual add
      void addToVariableActual(const ustring_t& id,
         const Tuple& k, HD *e);

      template <typename T>
      void buildConstantHD();

      protected:
      Parser::Header m_parseInfo;
      bool parseString(
         const ustring_t& string,
         const Parser::Spirit::stored_rule<Parser::scanner_t>& parser,
         const ustring_t& name = ustring_t());

      bool m_verbose;

      void addDimensions() {
         BOOST_FOREACH(const ustring_t& s, m_parseInfo.dimension_names) {
            m_dimTranslator.lookup(s);
         }
      }

      void cleanupParserObjects();

      //template <typename T>
      //void pushSystemParser(const T& parser) {
      //   m_systemParser.push(parser);
      //}

      Parser::EquationAdder m_equationAdder;

      Parser::Parsers m_parsers;
   };

   template <typename P>
   bool Interpreter::parseRange(
      Parser::iterator_t begin,
      Parser::iterator_t end,
      const P& parser,
      const ustring_t& name)
   {
      m_parseInfo.errorCount = 0;

      try {
         return Parser::Spirit::parse(
            begin, end,
            parser,
            Parser::skip_p
            ).
            full;
      } catch (const std::exception& e) {
         std::cerr << "caught std::exception parsing: " << e.what() << std::endl;
      } catch (...) {
         std::cerr << "unknown exception parsing" << std::endl;
         throw;
      }

      return false;
   }

   TypedValue hash(const TypedValue& dimension, const Tuple& context);
   TypedValue hash(size_t dimension, const Tuple& context);
}

#endif // INTERPRETER_HPP_INCLUDED
