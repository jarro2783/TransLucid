/* Typewriter implementation.
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

#include <tl/translator.hpp>
#include <tl/builtin_types.hpp>
#include <algorithm>
#include <tl/utility.hpp>

namespace TL = TransLucid;

int main(int argc, char *argv[])
{
  TL::Translator translator;
  TL::HD* h = translator.translate_expr(U"42");

  TL::TaggedConstant v = (*h)(TL::Tuple());
  std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;

  delete h;

  h = translator.translate_expr(U"«hello é world»");

  v = (*h)(TL::Tuple());
  std::u32string s = v.first.value<TL::String>().value();

  std::cout << TL::utf32_to_utf8(s) << std::endl;
  delete h;

  h = translator.translate_expr(U"\'è\'");
  v = (*h)(TL::Tuple());
  //std::cout << "char value = " << v.first.value<TL::Char>().value() << std::endl;
  s.clear();
  s += v.first.value<TL::Char>().value();
  std::cout << TL::utf32_to_utf8(s) << std::endl;

  delete h;
  h = translator.translate_expr(U"spdim");
  v = (*h)(TL::Tuple());
  std::cout << v.first.value<TL::Special>().value() << std::endl;

  h = translator.translate_expr(U"0X10");
  v = (*h)(TL::Tuple());
  std::cout << v.first.value<TL::Intmp>().value().get_ui() << std::endl;
  return 0;
}
