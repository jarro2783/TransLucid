#include "directory_system.hpp"
#include <tl/parser_fwd.hpp>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <tl/exception.hpp>
#include <tl/parser.hpp>

namespace TransLucid {

namespace DirectoryParser {

namespace Spirit = Parser::Spirit;

class DirectoryGrammar : public Spirit::grammar<DirectoryGrammar> {
   public:

   FileType fileType;

   DirectoryGrammar(Parser::Header& header,
      Parser::Parsers& parsers,
      Parser::EquationAdder& adder)
   : header(header),
   parsers(parsers),
   adder(adder)
   {}

   Parser::Header& header;
   Parser::Parsers& parsers;
   Parser::EquationAdder& adder;

   template <typename ScannerT>
   class definition {
      public:
      definition(DirectoryGrammar const& self)
      : self(self),
      equationGrammar(self.header, self.parsers, self.adder),
      headerGrammar(self.header, self.parsers)
      {
         equation_file = *(equationGrammar >> ";;") >> Spirit::end_p;
         header = headerGrammar;
      }

      Spirit::rule<ScannerT> const& start() const {
         switch (self.fileType) {
            case HEADER:
            return header;
            case EQNS:
            case STRUCTURE:
            case CLOCK:
            case THREAD:
            return equation_file;
            break;
         }
         return header;
      }

      private:
      DirectoryGrammar const& self;

      Spirit::rule<ScannerT> equation_file,
      header
      ;

      Parser::EquationGrammar equationGrammar;
      Parser::HeaderGrammar headerGrammar;
   };
};

DirectorySystem::DirectorySystem() {
   m_grammar = new DirectoryGrammar(m_parseInfo, m_parsers, m_equationAdder);
   #ifdef BOOST_SPIRIT_DEBUG
   BOOST_SPIRIT_DEBUG_GRAMMAR(*m_grammar);
   #endif
}

bool DirectorySystem::parseSystem(const ustring_t& path) {

   bool success = true;
   int currentThread = 0;
   int currentEqn = 0;

   std::string pathl = Glib::filename_from_utf8(path);

   typedef std::list<std::pair<std::string, DirectoryParser::FileType> > FileList;
   FileList files;
   files.push_back(std::make_pair(
      Glib::build_filename(pathl, "header"), HEADER));
   files.push_back(std::make_pair(
      Glib::build_filename(pathl, "clock"), CLOCK));
   files.push_back(std::make_pair(
      Glib::build_filename(pathl, "structure"), STRUCTURE));

   for (FileList::iterator iter = files.begin();
         iter != files.end() && success;
         ++iter)
   {
      Parser::EquationHolder eHolder(m_equationAdder);

      std::string file;
      try {
         file = iter->first;

         //if (parseString(contents, type, file)) {
         if (parseFile(file, iter->second)) {
            if (m_verbose) {
               std::clog << "successfully parsed " << file << std::endl;
            }
            switch (iter->second) {
               case HEADER:
               addDimensions();
               BOOST_FOREACH(const ustring_t& s, m_parseInfo.libraries) {
                  loadLibrary(s);
               }
               break;

               case CLOCK:
               setClock(eHolder.equations());
               break;

               case STRUCTURE:
               //put the equations files in the file list
               {
                  std::string namePre = pathl + "/eqns_";
                  BOOST_FOREACH(const Parser::equation_t& e, eHolder.equations()) {
                     if (e.get<0>() == L"eqns") {
                        ValueContext eqn = evaluate(e.get<2>(), Tuple());
                        if (eqn.first.index() == typeRegistry().indexIntmp()) {
                           const Intmp& value = eqn.first.value<Intmp>();
                           files.push_back(std::make_pair(namePre + value.value().get_str(), EQNS));
                           #warning add the equation to the system
                           //equationSets.push_back(createEquationSet(e.get<1>()));
                        }
                     }
                  }
               }
               //try to open thread files
               {
                  #if 0
                  std::string namePre = pathl + "/thread_";
                  int i = 0;
                  bool finished = false;
                  while (!finished) {
                     std::ostringstream os;
                     os << namePre << i;
                     std::string file = os.str();
                     std::ifstream is(file.c_str());
                     if (is) {
                        files.push_back(std::make_pair(file, THREAD));
                        threadSets.push_back(createEquationSet(EquationGuard()));
                     } else {
                        finished = true;
                     }

                     ++i;
                  }
                  #endif
                  std::ostringstream os;
                  os << pathl << "/demands";
                  std::string file = os.str();
                  std::ifstream is(file.c_str());
                  if (is) {
                     files.push_back(std::make_pair(file, THREAD));
                     #warning add demand to system
                     //threadSets.push_back(createEquationSet(EquationGuard()));
                  } else {
                     throw std::string("Could not open ") + file;
                  }
               }
               break;

               case EQNS:
               //eqnSets.at(currentEqn).second = m_parseInfo.equations;
               //addParsedEquationSet(m_parseInfo.equations);
               //m_parseInfo.equations.clear();
               {
                  #if 0
                  EquationSet& es = equationSets.at(currentEqn);
                  BOOST_FOREACH(
                     const Parser::equation_t& e,
                     eHolder.equations())
                  {
                     es.addEquation(
                        Equation(e.get<0>(), e.get<1>(), new ASTEquation(e.get<2>())));
                  }
                  ++currentEqn;
                  #endif
                  #warning have to completely redo this
               }
               break;

               case THREAD:
               {
                  #warning have to completely redo this
                  #if 0
                  EquationSet& es = threadSets.at(currentThread);
                  BOOST_FOREACH(const Parser::equation_t& e,
                     eHolder.equations())
                  {
                     es.addEquation(
                        Equation(e.get<0>(), e.get<1>(), new ASTEquation(e.get<2>())));
                  }
                  ++currentThread;
                  #endif
               }
               break;

               default:
               throw InternalError(
                  __FILE__ ": Interpreter::parseSystem() line: "
                  STRING_(__LINE__)
                  " should not have been reached");
            }

            cleanupParserObjects();
         } else {
            success = false;
            std::cerr << "failed parsing " << file << std::endl;
         }
      } catch (Glib::FileError& e) {
         std::cerr << e.what() << std::endl;
         ++m_parseInfo.errorCount;
         success = false;
      } catch (Glib::ConvertError& e) {
         std::cerr << "convert error reading " << file << std::endl;
      }
   }

   return success;
}

bool DirectorySystem::parseFile(const ustring_t& file, FileType type) {
   m_grammar->fileType = type;
   ustring_t contents = Glib::locale_to_utf8(Glib::file_get_contents(file));

   Parser::UIterator iter(contents);
   Parser::UIterator end = iter.make_end();

   return parseString(contents, *m_grammar);

#if 0
   return Spirit::parse(
      Parser::iterator_t(
         Parser::Iterator(iter),
         Parser::Iterator(end)),
      Parser::iterator_t(),
      *m_grammar,
      Parser::skip_p).full;
#endif
}

void DirectorySystem::addParsedEquationSet(const Parser::equation_v& eqns) {
}

//the only equation in the map should be clock
void DirectorySystem::setClock(const Parser::equation_v& equations) {
   const Parser::equation_t *e;
   if (equations.size() == 1 &&
      (e = &equations.front())->get<0>() == L"clock")
   {
      ValueContext r = evaluate(e->get<2>(), Tuple());
      if (r.first.index() != typeRegistry().indexIntmp()) {
         throw ParseError("clock variable must be an intmp");
      } else {
         const Intmp& i = r.first.value<Intmp>();
         m_maxClock = i.value();
      }
   } else {
      throw ParseError("Clock variable not set correctly");
   }
}

}

}
