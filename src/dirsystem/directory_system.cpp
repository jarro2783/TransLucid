/* Directory system.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#if 0

#include "directory_system.hpp"
#include <tl/parser_fwd.hpp>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <tl/exception.hpp>
#include <tl/parser.hpp>
#include <tl/expr_compiler.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{

namespace DirectoryParser
{

//namespace Spirit = Parser::Spirit;
namespace qi = boost::spirit::qi;

namespace
{

template <typename Parser, typename Action, typename Next>
class parse_file_function
{
  public:

  parse_file_function
  (
    const std::string& f,
    Parser& p,
    Action& a,
    Next& n
  )
  : file(f),
    parse(p),
    action(a),
    next(n)
  {}

  void operator()() const
  {
    parse();
    action(parse.result());
    next();
  }

  private:

  std::string file;
  const Parser& parse;
  const Action& action;
  const Next& next;
};

template <typename Parser, typename Result>
class parse_one_file_function
{
  public:

  parse_one_file_function
  (
    const std::string& file,
    Parser& p,
    Result& r
  )
  : m_file(file),
    m_parser(p),
    m_result(r)
  {
  }

  void
  operator()() const
  {
    std::wstring text;
    std::wstring::const_iterator pos = text.begin();
    qi::phrase_parse
    (
      pos,
      text.cend(),
      m_parser,
      qi::standard_wide::space,
      m_result
    );
  }

  const Result
  result() const
  {
    return m_result;
  }

  private:
  std::string m_file;
  Parser& m_parser;
  Result& m_result;
};

}

#if 0
class DirectoryGrammar : public qi::grammar<DirectoryGrammar>
{
  public:

  FileType fileType;

  DirectoryGrammar
  (
    Parser::Header& header,
    Parser::Parsers& parsers,
    Parser::EquationAdder& adder
  )
  : header(header),
    parsers(parsers),
    adder(adder)
  {}

  Parser::Header& header;
  Parser::Parsers& parsers;
  Parser::EquationAdder& adder;

  template <typename ScannerT>
  class definition
  {
    public:
    definition(DirectoryGrammar const& self)
    : self(self),
      equationGrammar(self.header, self.parsers, self.adder),
      headerGrammar(self.header, self.parsers)
    {
      equation_file = *(equationGrammar >> ";;") >> Spirit::end_p;
      header = headerGrammar;
    }

    Spirit::rule<ScannerT> const&
    start() const
    {
      switch (self.fileType)
      {
        case HEADER:
          return header;
        case EQNS:
        case STRUCTURE:
        case CLOCK:
        case DEMANDS:
          return equation_file;
          break;
      }
      return header;
    }

    private:
    DirectoryGrammar const& self;

    Spirit::rule<ScannerT> equation_file;
    Spirit::rule<ScannerT> header;

    Parser::EquationGrammar equationGrammar;
    Parser::HeaderGrammar headerGrammar;
   };
};
#endif

DirectorySystem::DirectorySystem()
: m_compiler(m_interpreter), m_expr_parser(m_header)
{
  m_expr_parser.set_context_perturb(m_tuple_parser);
  m_tuple_parser.set_expr(m_expr_parser);
}

bool
DirectorySystem::parseSystem(const ustring_t& path)
{

  bool success = true;

  std::string pathl = Glib::filename_from_utf8(path);

  typedef std::list<std::pair<std::string, DirectoryParser::FileType>>
          FileList;
  FileList files;
  files.push_back(std::make_pair
    (Glib::build_filename(pathl, "header"), HEADER));
  files.push_back(std::make_pair
    (Glib::build_filename(pathl, "clock"), CLOCK));
  files.push_back(std::make_pair
    (Glib::build_filename(pathl, "eqns"), EQNS));
  files.push_back(std::make_pair
    (Glib::build_filename(pathl, "demands"), DEMANDS));

  const std::wstring headerText = L"dimension<test>;;";

  std::wstring::const_iterator pos;

  for (FileList::iterator iter = files.begin();
       iter != files.end() && success; ++iter)
  {
      //Parser::EquationHolder eHolder(m_equationAdder);

    switch (iter->second)
    {
      case HEADER:
      pos = headerText.begin();
      qi::phrase_parse
      (
        pos,
        headerText.end(),
        m_header_parser,
        qi::standard_wide::space,
        m_header
      );

      case EQNS:
      case DEMANDS:
      {
        std::vector<Parser::ParsedEquation> eqns;
      }
    }

    #if 0
    std::string file;
    try
    {
      file = iter->first;

      //if (parseString(contents, type, file))
      if (parseFile(file, iter->second))
      {
        if (m_verbose)
        {
          std::clog << "successfully parsed " << file << std::endl;
        }
        switch (iter->second)
        {
          case HEADER:
            addDimensions();
            BOOST_FOREACH(const ustring_t& s, m_parseInfo.libraries)
            {
              loadLibrary(s);
            }
            break;

          case CLOCK:
            setClock(eHolder.equations());
            break;

          case EQNS:
          case DEMANDS:
          {
            //demands and equations are the same thing
            size_t dim_id = DIM_ID;
            size_t dim_valid = get_dimension_index(this, "_validguard");
            BOOST_FOREACH(const Parser::equation_t& e, eHolder.equations())
            {
              tuple_t k;
              k.insert(std::make_pair(dim_id,
              TypedValue(String(std::get<0>(e)), TYPE_INDEX_USTRING)));

              //need to compile the guard
              HD* guardTuple = m_compiler.compile(std::get<0>(std::get<1>(e)));
              HD* guardBool = m_compiler.compile(std::get<1>(std::get<1>(e)));
              k.insert(std::make_pair(dim_valid,
                TypedValue(EquationGuardType(EquationGuard(guardTuple, guardBool)),
                TYPE_INDEX_GUARD)));

              HD* h = m_compiler.compile(std::get<2>(e));
              addExpr(Tuple(k), h);
            }
            break;
          }

          default:
            throw InternalError(
              __FILE__ ": Interpreter::parseSystem() line: "
              STRING_(__LINE__)
              " should not have been reached");
        }

        cleanupParserObjects();
      }
      else
      {
        success = false;
        std::cerr << "failed parsing " << file << std::endl;
      }
    }
    catch (Glib::FileError& e)
    {
       std::cerr << e.what() << std::endl;
       ++m_parseInfo.errorCount;
       success = false;
    }
    catch (Glib::ConvertError& e)
    {
       std::cerr << "convert error reading " << file << std::endl;
    }
    #endif
  }

  return success;
}

#if r
bool
DirectorySystem::parseFile(const ustring_t& file, FileType type)
{
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
#endif

void
DirectorySystem::addParsedEquationSet(const Parser::equation_v& eqns)
{
}

//the only equation in the map should be clock
void
DirectorySystem::setClock(const Parser::equation_v& equations)
{
  #if 0
  const Parser::equation_t* e;
  if (equations.size() == 1 &&
      std::get<0>(*(e = &equations.front())) == L"clock")
  {
    HD* h = m_compiler.compile(std::get<2>(*e));
    TypedValue v = (*h)(Tuple()).first;//= evaluate(e->get<2>(), Tuple());
    if (v.index() != TYPE_INDEX_INTMP)
    {
      throw ParseError("clock variable must be an intmp");
    }
    else
    {
      const Intmp& i = v.value<Intmp>();
      m_maxClock = i.value();
    }
  }
  else
  {
    throw ParseError("Clock variable not set correctly");
  }
  #endif
}

void
DirectorySystem::addLibrarySearchPath(const std::string& path)
{
  m_lt.addSearchPath(path);
}

}

}

#endif
