#ifndef DIRECTORY_SYSTEM_HPP_INCLUDED
#define DIRECTORY_SYSTEM_HPP_INCLUDED

#include <tl/interpreter.hpp>
#include <tl/parser_fwd.hpp>
#include <tl/builtin_types.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/utility.hpp>
#include <tl/parser.hpp>
#include <tl/tuple_parser.hpp>
#include <tl/expr_parser.hpp>

namespace TransLucid {

   namespace DirectoryParser {
      enum FileType {
         HEADER,
         EQNS,
         STRUCTURE,
         CLOCK,
         DEMANDS
      };

      class DirectoryGrammar;

      class DirectorySystem {
         public:
         DirectorySystem();

         bool parseSystem(const ustring_t& path);

         template <typename OutputIterator>
         void evaluateSystem(OutputIterator out);

         void addLibrarySearchPath(const std::string& path);

         private:
         void addParsedEquationSet(const Parser::equation_v& eqns);
         void setClock(const Parser::equation_v& equations);

         bool parseFile(const ustring_t& file, FileType type);

         mpz_class m_maxClock;

         DirectoryGrammar *m_grammar;

         ExprCompiler m_compiler;

         Libtool m_lt;

         TransLucid::Interpreter m_interpreter;

         Parser::HeaderGrammar<Parser::iterator_t> m_header_parser;
         Parser::ExprGrammar<Parser::iterator_t> m_expr_parser;
         Parser::TupleGrammar<Parser::iterator_t> m_tuple_parser;
         Parser::Header m_header;
      };

      template <typename OutputIterator>
      void DirectorySystem::evaluateSystem(OutputIterator out) {

         using boost::assign::map_list_of;

         //std::cout << "list of variables" << std::endl;
         //listVariables();

         //size_t dimTime = dimTranslator().lookup("time");
         size_t dimTime = DIM_TIME;
         //size_t dim_id = dimTranslator().lookup("id");
         size_t dim_id = DIM_ID;
         //TypedValue demandString(String("demand"), typeRegistry().indexString());
         TypedValue demandString = generate_string(U"demand");

         //evaluate from time 1 to end
         for (size_t time = 1; time <= m_maxClock; ++time) {
            //just do one thread
            tuple_t tuple = map_list_of(dimTime, TypedValue(Intmp(time), TYPE_INDEX_INTMP))
            (dim_id, demandString);
            Tuple c(tuple);

            //Equation e = findEquation("demand", c);
            //Variable *v = lookupVariable("demand");
            *out = m_interpreter(c);
            #if 0
            Variable *v = 0;

            if (v) {
               //*out = e.equation()->evaluate(*this, c);
               *out = (*v)(c);
            } else {
               *out = ValueContext(TypedValue(Special(Special::UNDEF), typeRegistry().indexSpecial()), c);
            }
            #endif
            ++out;
         }
      }
   }
}

#endif // DIRECTORY_SYSTEM_HPP_INCLUDED
