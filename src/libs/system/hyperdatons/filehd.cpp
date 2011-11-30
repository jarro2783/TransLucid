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

/**
 * @file filehd.cpp
 * File hyperdaton implementation.
 */

#include <tl/hyperdatons/filehd.hpp>
#include <tl/system.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/string.hpp>
#include <tl/utility.hpp>

#include <tl/parser_iterator.hpp>
#include "tl/lexertl.hpp"

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

  //first read in the whole file
  std::string content = read_file(in);

  Parser::StreamPosIterator begin(Parser::makeUTF8Iterator(content.begin()));
  Parser::StreamPosIterator end(Parser::makeUTF8Iterator(content.end()));
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

Constant
FileArrayOutFn::applyFn(const std::vector<Constant>& args) const
{  
  if 
  (
    args.size() == 3 && 
    args[0].index() == TYPE_INDEX_USTRING &&
    args[1].index() == TYPE_INDEX_INTMP &&
    args[2].index() == TYPE_INDEX_INTMP
  )
  {
    try
    {
      return Types::Hyperdatons::create
      (
        new FileArrayOutHD
        (
          Types::String::get(args[0]), 
          Types::Intmp::get(args[1]),
          Types::Intmp::get(args[2]),
          m_system
        ),
        TYPE_INDEX_OUTHD
      );
    }
    catch(...)
    {
      return Types::Special::create(SP_CONST);
    }
  }
  else
  {
    return Types::Special::create(SP_CONST);
  }
}

Constant
FileArrayOutFn::applyFn(const Constant& arg) const
{
  return Constant();
}

//for now we will just fill it with some random data to see what happens
Tuple
FileArrayInHD::variance() const
{
  return m_array->variance();
}

Constant
FileArrayInHD::get(const Context& k) const
{
  return m_array->get(k);
}


FileArrayOutHD::FileArrayOutHD
(
  const u32string& file, 
  const mpz_class& height,
  const mpz_class& width,
  System& system
)
: OutputHD(1), m_height(height.get_ui()), m_width(width.get_ui())
{
  m_file.open(utf32_to_utf8(file));

  if (!m_file.is_open())
  {
    throw "Could not open output file";
  }

  m_array = new ArrayNHD<mpz_class, 2>
  (
    {m_height, m_width},
    {Types::Dimension::create(DIM_ARG0), Types::Dimension::create(DIM_ARG1)},
    system,
    static_cast<Constant(*)(const mpz_class&)>(&Types::Intmp::create),
    &Types::Intmp::get
  );
}

Tuple
FileArrayOutHD::variance() const
{
  return m_array->variance();
}

void
FileArrayOutHD::commit()
{
  //write to file
  for (size_t i = 0; i != m_height; ++i)
  {
    for (size_t j = 0; j != m_width; ++j)
    {
      m_file << (*m_array)[i][j] << " ";
    }
    m_file << std::endl;
  }
}

void
FileArrayOutHD::put(const Context& t, const Constant& c)
{
  if (c.index() == TYPE_INDEX_INTMP)
  {
    m_array->put(t, c);
  }
}

class FileInCreateWS : public WS
{
  public:
  FileInCreateWS(System& s)
  : m_system(s)
  {}

  Constant
  operator()(Context& k)
  {
    return Types::BaseFunction::create(FileArrayInFn(m_system));
  }

  private:
  System& m_system;
};

class FileOutCreateWS : public WS
{
  public:
  FileOutCreateWS(System& s)
  : m_system(s)
  {}

  Constant
  operator()(Context& k)
  {
    return Types::BaseFunction::create(FileArrayOutFn(m_system));
  }

  private:
  System& m_system;
};

//the following functions will create the appropriate file hyperdaton
//which contains an appropriate fstream object and then wrap it up
//as a constant
#if 0
inline Constant
open_input_file(type_index ti, const u32string& file)
{
  FileInputHD* in = new FileInputHD(file);

  return Types::Hyperdatons::create(in, ti);
}

inline Constant
open_output_file(type_index ti, const u32string& file)
{
  //TODO implement me
  return Types::Special::create(SP_ERROR);
}

inline Constant
open_io_file(type_index ti, const u32string& file)
{
  //TODO implement me
  return Types::Special::create(SP_ERROR);
}

struct FileOpener
{
  FileOpener(type_index in, type_index out, type_index io)
  : m_in(in), m_out(out), m_io(io)
  {
  }

  Constant
  operator()(const Constant& file, const Constant& mode)
  {
    if (mode.index() != TYPE_INDEX_INTMP || 
        file.index() != TYPE_INDEX_USTRING)
    {
      return Types::Special::create(SP_TYPEERROR);
    }

    const mpz_class& intmode = get_constant_pointer<mpz_class>(mode);
    const u32string& sfile = get_constant_pointer<u32string>(file);

    switch (intmode.get_ui())
    {
      case 1:
      //input
      return open_input_file(m_in, sfile);
      break;

      case 2:
      //output
      return open_output_file(m_out, sfile);
      break;

      case 3:
      //io
      return open_io_file(m_out, sfile);
      break;

      default:
      return Types::Special::create(SP_DIMENSION);
    }
  }

  type_index m_in, m_out, m_io;
};
#endif


void
init_file_hds(System& s)
{
  //don't know about this stuff
  //this is probably the old wrong stuff
  //type_index in, out, io;

  //in = s.getTypeIndex(U"inhd");
  //out = s.getTypeIndex(U"outhd");
  //io = s.getTypeIndex(U"iohd");

  //s.registerFunction(U"fileopen", 
  //  make_function_type<2>::type(
  //    FileOpener(in, out, io)
  //  )
  //);
  //don't know about above

  s.addEquation(U"file_array_in_hd", new FileInCreateWS(s));
  s.addEquation(U"file_array_out_hd", new FileOutCreateWS(s));
}

}
