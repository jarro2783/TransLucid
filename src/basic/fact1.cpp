/* Factorial example.
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

using namespace TransLucid;

int main(int argc, char* argv[])
{
  HD* e = 0;
  try {
    Translator t;

    t.parse_header
    (
      U"dimension ustring<n>;;"
      U"infixl ustring<-> ustring<operator-> 5;;"
      U"infixl ustring<*> ustring<operator*> 10;;"
      U"library ustring<int>;;"
    );

    t.translate_and_add_equation_set
    (
      U"fact | [n:0] = 1;;"
      U"fact = #n * (fact @ [n:#n-1]);;"
    );

    e = t.translate_expr(U"fact @ [n:20]");

    TaggedConstant result = (*e)(Tuple());

    std::cout << result.first.value<Intmp>().value() << std::endl;
  } catch (const char* c) {
    std::cerr << "Terminated with exception: " << c << std::endl;
  } catch (std::string& s) {
    std::cerr << "Terminated with exception: " << s << std::endl;
  } catch (std::exception& e) {
    std::cerr << "Terminated with exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Terminated with unknown exception" << std::endl;
  }

  delete e;

  return 0;
}
