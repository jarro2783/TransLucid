/* Environment variable hyperdaton.
   Copyright (C) 2011 Jarryd Beck.

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

#include <tl/charset.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/hyperdatons/envhd.hpp>
#include <tl/types/special.hpp>
#include <tl/types/string.hpp>
#include <tl/types/type.hpp>
#include <tl/types_util.hpp>

namespace TransLucid
{

EnvHD::EnvHD
(
  DimensionRegistry& dimReg,
  TypeRegistry& typeReg
)
: InputHD(1)
, m_dimVariable(dimReg.getDimensionIndex(U"envvar"))
, m_variance(tuple_t
    {
      {m_dimVariable,
        Types::Type::create(typeReg.getTypeIndex(U"ustring"))
      }
    })
{
}

Constant
EnvHD::get(const Context& index) const
{
  const Constant& v = index.lookup(m_dimVariable);
  if (v.index() == TYPE_INDEX_USTRING)
  {
    const u32string& s = get_constant_pointer<u32string>(v);
    char* c = getenv(std::string(s.begin(), s.end()).c_str());

    return Types::String::create(chars_to_u32string(c));
  }
  else
  {
    //this should never happen because the system guarantees the preconditions
    //with bestfitting
    throw "EnvHD: index invalid: " __FILE__ ": " STRING_(__LINE__);
  }
}

Tuple
EnvHD::variance() const
{
  return m_variance;
}

}
