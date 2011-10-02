/* tltext web app main file.
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

#include "tltext.hpp"

#include <sstream>
#include <iostream>

//This runs the main web app interface to tltext.
//We are using the POST method for html.

//we'll set up tltext to read and write to and from
//i/ostringstream
//then output appropriately

int main(int argc, char* argv[])
{
  std::cout << "Content-type: text/html\r\n\r\n";

  //get the input length
  char* lengthstr = getenv("CONTENT_LENGTH");
  if (lengthstr == nullptr)
  {
    std::cout << "Empty input string" << std::endl;
    exit(1);
  }
  int length = atoi(lengthstr) + 1;

  std::unique_ptr<char[]> inbuf(new char[length]);
  std::cin.get(inbuf.get(), length);

  std::istringstream input(inbuf.get());

  std::cout << "<html><head><title>tl program</title></head><body>";

  std::cout << inbuf.get() << std::endl;

  std::cout << "</body></html>" << std::endl;

  //std::vector<std::pair<std::string, std::string>> invars;

  //std::string var;
  //std::getline(input, var, '=');

  return 0;
}
