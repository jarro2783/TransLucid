/* TODO: Give a descriptor.
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

#ifdef TL_DEBUG
  #define BOOST_SPIRIT_DEBUG
#endif

#include "parser.hpp"
#include "interactive.hpp"
#include <iostream>
#include <iterator>
#include <boost/spirit/include/classic_multi_pass.hpp>
#include <tl/exception.hpp>
#include <tl/parser.hpp>
#include <glibmm/fileutils.h>
#include <boost/assign.hpp>

namespace TLInteractive
{

  namespace
  {

    namespace Parser = TL::Parser;
    namespace Spirit = Parser::Spirit;

    class StreamIterator : public Parser::IteratorTraits
    {
      public:
      typedef wchar_t result_type;
      StreamIterator(std::istream& is)
      : m_is(&is)
      {}

      StreamIterator()
      : m_is(0)
      {}

      result_type
      operator()() const
      {
        if (!m_is)
        {
          return -1;
        }

        int c = m_is->get();

        //the number of characters extra to read
        int readAmount;
        if (c == -1)
        {
          return -1;
        }
        else if ((c & 0xfc) == 0xfc)
        {
          readAmount = 5;
        }
        else if ((c & 0xf8) == 0xf8)
        {
          readAmount = 4;
        }
        else if ((c & 0xf0) == 0xf0)
        {
          readAmount = 3;
        }
        else if ((c & 0xe0) == 0xe0)
        {
          readAmount = 2;
        }
        else if ((c & 0xc0) == 0xc0)
        {
          readAmount = 1;
        }
        else if ((c <= 0x7f))
        {
          readAmount = 0;
        }
        else
        {
          throw TL::ParseError("invalid byte sequence in input");
        }

        char buf[6] = {c};

        int current = 1;
        int i = 0;
        while (i < readAmount)
        {
          buf[current] = m_is->get();
          ++i;
          ++current;
        }

        wchar_t w;
        if (mbstowcs(&w, buf, 1) == size_t(-1))
        {
          throw TL::ParseError
            ("invalid byte senquence in input converting character");
        }
        return w;
      }

      static result_type eof;

      private:
      std::istream* m_is;
    };

    StreamIterator::result_type StreamIterator::eof = -1;

    class ParserIterator : public Parser::IteratorBase
    {
      private:
      typedef Spirit::multi_pass
      <
        StreamIterator,
        Spirit::multi_pass_policies::functor_input
      > iterator_t;
      public:

      ParserIterator()
      : m_iter(new iterator_t(StreamIterator()))
      {}

      ParserIterator(const ParserIterator& other)
      {
        if (other.m_iter)
        {
          m_iter = new iterator_t(*other.m_iter);
        }
        else
        {
          m_iter = 0;
        }
      }

      ParserIterator(std::istream& is)
      {
        StreamIterator iter(is);
        m_iter = new iterator_t(iter);
      }

      ~ParserIterator()
      {
        delete m_iter;
      }

      ParserIterator* clone() const
      {
        return new ParserIterator(*this);
      }

      const wchar_t&
      dereference() const
      {
        return **m_iter;
      }

      void
      increment()
      {
        ++(*m_iter);
      }

      bool
      equal(const IteratorBase& rhs) const
      {
        const ParserIterator* r =
          dynamic_cast<const ParserIterator*>(&rhs);
        if (r)
        {
          if (m_iter && r->m_iter)
          {
            return *m_iter == *r->m_iter;
          }
          else if (m_iter == 0 && r->m_iter == 0)
          {
            return true;
          }
          else
          {
            return false;
          }
        }
        else
        {
          return false;
        }
      }

      private:
      iterator_t* m_iter;
    };

  }

void
System::run()
{
  time = 1;
  std::cout << "time: 1" << std::endl;
  Grammar g(m_parsers, m_parseInfo, m_equationAdder, *this);
  //BOOST_SPIRIT_DEBUG_GRAMMAR(g);
  Parser::EquationHolder holder(m_equationAdder);

  Signals::connection c1 = g.postParseInput.connect
    (boost::bind(&System::postInputSignal, this, _1));
  //Signals::connection c2 = g.postParseEquation.connect
  //  (boost::bind(&System::postEqnSignal, this, holder.equations()));

  Parser::Iterator iter(ParserIterator(std::cin));
  Parser::Iterator end = ParserIterator();

  parseRange(Parser::iterator_t(iter, end), Parser::iterator_t(), g);

  #if 0
  Parser::iterator_t pbegin(iter, end);
  Parser::iterator_t pend;

  while (iter != end)
  {
    std::cout << *iter;
    ++iter;
  }
  #endif
}
void
System::postInputSignal(std::vector<TL::AST::Expr*> const& ev)
{

  using boost::assign::map_list_of;

  size_t dimTime = dimTranslator().lookup("time");

  TL::tuple_t tuple =
    map_list_of(dimTime,
    TL::TypedValue(TL::Intmp(time), typeRegistry().indexIntmp()));
  TL::Tuple c(tuple);

  TL::AST::Expr* lastExpr = 0;

  BOOST_FOREACH(TL::AST::Expr* e, ev)
  {
    TL::TaggedValue v = evaluate(e, c);
    TL::TypeRegistry& reg = typeRegistry();
    const TL::TypeManager* m = reg.findType(v.first.index());
    m->print(std::cout, v.first, v.second);
    std::cout << std::endl;
    lastExpr = e;
  }

  if (lastExpr != 0)
  {
    demands.addEquation
      (TL::Equation("demand",
                    TL::EquationGuard(c),
                    new TL::ASTEquation(lastExpr)));
  }

  ++time;

  std::cout << "time: " << time << std::endl;
}

void
System::parseHeader(const std::string& file)
{
  TL::Parser::HeaderGrammar headerParser(m_parseInfo, m_parsers);

  //TL::ustring_t contents =
  //  Glib::locale_to_utf8(Glib::file_get_contents(file));

  Parser::UIterator iter(contents);
  Parser::UIterator end = iter.make_end();

  parseString(contents, headerParser);

  BOOST_FOREACH(const TL::u32string& s, m_parseInfo.libraries)
  {
    std::cout << "loading library \"" << s << "\"" << std::endl;
    loadLibrary(s);
  }
}

}

#endif
