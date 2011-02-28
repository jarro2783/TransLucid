/* TransLucid lexer utility functions.
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

#include <gmpxx.h>

namespace TransLucid
{
  namespace Parser
  {
    #if 0
    mpq_class
    init_mpq
    (
      const std::wstring::const_iterator& begin, 
      const std::wstring::const_iterator& end,
      int base
    );
    #endif

    template <typename Iterator>
    mpq_class
    init_mpq
    (
      const Iterator& begin,
      const Iterator& end,
      int base
    );

    template <typename Iterator>
    mpf_class
    init_mpf
    (
      const Iterator& begin,
      const Iterator& end,
      int base
    );

    #if 0
    mpf_class
    init_mpf
    (
      const std::wstring::const_iterator& begin,
      const std::wstring::const_iterator& end,
      int base
    );
    #endif
  }
}
