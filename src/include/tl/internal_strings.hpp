/* Predefined strings used internally.
   Copyright (C) 2011 Jarryd Beck

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

#include <tl/types.hpp>
#include <unordered_map>

namespace TransLucid
{
  //dimensions
  extern u32string type_name_dim;
  extern u32string text_dim;
  extern u32string fnname_dim;

  //identifiers
  extern u32string LITERAL_IDENT;
  extern u32string FN1_IDENT;
  extern u32string FN2_IDENT;
  extern u32string PRINT_IDENT;

  //type names
  extern u32string typename_intmp;
  extern u32string typename_ustring;
  extern u32string typename_inhd;
  extern u32string typename_outhd;
  extern u32string typename_iohd;

  extern std::unordered_map<u32string, Constant> string_constants;
}
