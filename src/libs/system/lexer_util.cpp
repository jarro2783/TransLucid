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

#include <tl/lexer_util.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

namespace TransLucid { namespace Parser { //namespace detail {

template <typename Iterator>
mpq_class
init_mpq(const Iterator& begin, const Iterator& end, int base)
{
  std::string s;
  s.reserve(end - begin);

  Iterator current = begin;
  while (current != end)
  {
    if (*current == '_')
    {
      s += '/';
    }
    else
    {
      s += *current;
    }
    ++current;
  }

  mpq_class value(s, base);

  return value;
}

template <typename Iterator>
mpf_class
init_mpf(const Iterator& begin, const Iterator& end, int base)
{
  unsigned int prec = mpf_get_default_prec();
  std::cerr << "default precision is " << prec << std::endl;
  Iterator current = begin;
  std::string s;
  s.reserve(end - begin);

  bool find_precision = false;
  while (current != end && !find_precision)
  {
    if (*current == '^')
    {
      s += '@';
    }
    else if (*current == '#')
    {
      find_precision = true;
    }
    else
    {
      s += *current;
    }
    ++current;
  }

  if (find_precision)
  {
    mpz_class precision(std::string(current, end), base);
    prec = precision.get_si();
  }

  std::cerr << "init_mpf: " << s << ", base: " << base 
    << ", precision " << prec << std::endl;
  return mpf_class(s, prec, base);
}

template mpf_class init_mpf<std::wstring::const_iterator>
(
  const std::wstring::const_iterator&,
  const std::wstring::const_iterator&,
  int
);

#if 0
template mpf_class init_mpf<std::wstring::iterator>
(
  const std::wstring::iterator&,
  const std::wstring::iterator&,
  int
);
#endif

template mpq_class init_mpq<std::wstring::const_iterator>
(
  const std::wstring::const_iterator&,
  const std::wstring::const_iterator&,
  int
);

#if 0
template mpq_class init_mpq<std::wstring::iterator>
(
  const std::wstring::iterator&,
  const std::wstring::iterator&,
  int
);
#endif
 
} }
