/* Parser functions.
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

/** @file src/libs/system/parser.cpp
 * Miscellaneous parser utility.
 */

#include <tl/builtin_types.hpp>
#include <tl/charset.hpp>
#include <tl/output.hpp>
#include <tl/parser_header.hpp>
#include <tl/tree_printer.hpp>
#include <tl/parser_util.hpp>
#include <tl/parser_api.hpp>

namespace TransLucid
{

namespace Parser
{

std::ostream&
operator<<(std::ostream& os, const Equation& eqn)
{
  os << "Equation(" << std::get<0>(eqn) << ")" << std::endl;
  return os;
}

std::ostream&
operator<<(std::ostream& os, const std::pair<Equation, DeclType>& p)
{
  os << "Declaration " << p.second << ": " << p.first << std::endl;
  return os;
}

Header::Header()
{
}

std::ostream& 
operator<<(std::ostream& os, const Header& h)
{
  os << "header";
  return os;
}

}

}
