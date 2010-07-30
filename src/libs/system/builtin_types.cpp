/* Built-in types.
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

#include <tl/builtin_types.hpp>
#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <tl/utility.hpp>

using boost::assign::map_list_of;

namespace TransLucid
{

const Special::StringValueInitialiser Special::m_sv;

Special::StringValueInitialiser::StringValueInitialiser()
: vtos
{
  {Special::ERROR, U"error"},
  {Special::ACCESS, U"access"},
  {Special::TYPEERROR, U"type"},
  {Special::DIMENSION, U"dim"},
  {Special::UNDEF, U"undef"},
  {Special::CONST, U"const"},
  {Special::LOOP, U"loop"},
  {Special::MULTIDEF, U"multidef"}
}
{
  std::basic_string<unsigned int> prefix({'s', 'p'});
  BOOST_FOREACH(ValueStringMap::value_type const& v, vtos)
  {
    stov.insert(std::make_pair(v.second, v.first));
    std::basic_string<unsigned int> parser_string = prefix + 
      std::basic_string<unsigned int>(v.second.begin(), v.second.end());
    //std::cerr << "mapping " << utf32_to_utf8(to_u32string(parser_string))
    //  << " to " << v.first << std::endl;
    parser_stov.insert(std::make_pair(parser_string, v.first));
  }
}

void
String::print(std::ostream& os) const
{
  os << "ustring<" << utf32_to_utf8(m_s) << ">";
}

void
Special::print(std::ostream& os) const
{
  os << "special<" << utf32_to_utf8(m_sv.vtos.find(m_v)->second) << ">";
}

void Char::print(std::ostream& os) const
{
  u32string s(1, m_c);
  os << "uchar<" << utf32_to_utf8(s) << ">";
}

}
