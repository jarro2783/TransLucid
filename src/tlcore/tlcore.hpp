/* Core TransLucid application header.
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

#include <tl/parser_fwd.hpp>
#include <tl/system.hpp>
#include <iostream>

namespace TransLucid
{
  namespace Parser
  {
    template <typename Iterator>
    class SkipGrammar;
  }

  namespace TLCore
  {
    template <typename Iterator>
    class Grammar;

    class TLCore
    {
      public:
      TLCore();

      void 
      verbose(bool v)
      {
        m_verbose = v;
      }

      void 
      reactive(bool r)
      {
        m_reactive = r;
      }

      void 
      run();

      void 
      set_input(std::istream* is)
      {
        m_is = is;
      }

      void 
      set_output(std::ostream* os)
      {
        m_os = os;
      }

      private:
      bool m_verbose;
      bool m_reactive;
      Grammar<Parser::string_type::const_iterator>* m_grammar;
      Parser::SkipGrammar<Parser::string_type::const_iterator>* m_skipper;      

      std::istream* m_is;
      std::ostream* m_os;

      SystemHD m_system;
      std::vector<HD*> m_exprs;

      std::u32string 
      read_input();
    };
  }
}
