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

#include <tl/internal_strings.hpp>
#include <tl/types.hpp>
#include <tl/types/string.hpp>

namespace TransLucid
{
  //dims
  u32string type_name_dim = U"typename";
  u32string text_dim = U"text";
  u32string fnname_dim = U"fnname";

  //identifiers
  u32string LITERAL_IDENT = U"LITERAL";
  u32string FN1_IDENT = U"FN1";
  u32string FN2_IDENT = U"FN2";
  u32string PRINT_IDENT = U"PRINT";

  //type names
  u32string typename_intmp = U"intmp";
  u32string typename_ustring = U"ustring";
  u32string typename_inhd = U"inhd";
  u32string typename_outhd = U"outhd";
  u32string typename_iohd = U"iohd";

  std::unordered_map<u32string, Constant> string_constants;
}
