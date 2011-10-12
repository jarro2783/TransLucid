/* FileIO hyperdaton functions.
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

#include <tl/hyperdatons/filehd.hpp>
#include <tl/system.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/string.hpp>

#include <fstream>

namespace TransLucid
{

FileArrayInHD::FileArrayInHD(const u32string& file, System& s)
: InputHD(0)
{
  std::ifstream in(utf32_to_utf8(file).c_str());

  if (!in.is_open())
  {
    throw "Could not open file";
  }

  size_t height;
  size_t maxWidth = 0;

  //simply read everything in each line and stuff it into an array
  std::vector<std::vector<mpz_class>> inArray;
  while (in)
  {
    std::string line;
    std::getline(in, line);

    std::istringstream linestream(line);
    std::vector<mpz_class> lineArray;
    while (linestream)
    {
      mpz_class v;
      linestream >> v;
      lineArray.push_back(v);
    }
    if (lineArray.size() > maxWidth)
    {
      maxWidth = lineArray.size();
    }
    inArray.push_back(lineArray);
  }
  height = inArray.size();

  m_array = new ArrayNHD<mpz_class, 2>
  (
    {height, maxWidth},
    {Types::Dimension::create(DIM_ARG0), Types::Dimension::create(DIM_ARG1)},
    s,
    static_cast<Constant(*)(const mpz_class&)>(&Types::Intmp::create),
    &Types::Intmp::get
  );

  size_t i = 0, j;
  for (auto outer = inArray.begin(); outer != inArray.end(); ++outer)
  {
    j = 0;
    for (auto inner = outer->begin(); inner != outer->end(); ++inner)
    {
      (*m_array)[i][j] = *inner;
      ++j;
    }
    ++i;
  }
}

Constant
FileArrayInFn::applyFn(const std::vector<Constant>& args) const
{
  return Constant();
}

Constant
FileArrayInFn::applyFn(const Constant& arg) const
{
  if (arg.index() == TYPE_INDEX_USTRING)
  {
    try
    {
      return Types::Hyperdatons::create
      (
        new FileArrayInHD(Types::String::get(arg), m_system),
        TYPE_INDEX_INHD
      );
    }
    catch (...)
    {
      //something went wrong, couldn't open the file, bad format or something
      return Types::Special::create(SP_CONST);
    }
  }
  else
  {
    return Types::Special::create(SP_CONST);
  }
}

//for now we will just fill it with some random data to see what happens
Tuple
FileArrayInHD::variance() const
{
  return m_array->variance();
}

Constant
FileArrayInHD::get(const Tuple& k) const
{
  //TODO check this out with SFINAE problem
  return static_cast<InputHD*>(m_array)->get(k);
}

}
