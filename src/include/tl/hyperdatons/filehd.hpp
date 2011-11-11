/* File hyperdatons.
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

/**
 * @file filehd.hpp
 * File hyperdaton header.
 */

#include <tl/charset.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/hyperdatons/arrayhd.hpp>
#include <tl/types_util.hpp>
#include <tl/types/function.hpp>

#include <fstream>

namespace TransLucid
{
  #if 0
  class FileInputHD : public InputHD
  {
    public:
    FileInputHD(const u32string& file)
    : InputHD(1)
    {
      std::string utf8_f = utf32_to_utf8(file);

      m_stream.open(utf8_f.c_str());

      mpz_class left(0);
      //this is 2^63 - 1
      mpz_class right("9223372036854775807");
      Range r(&left, &right); 

      m_variance = tuple_t{{DIM_ZERO,Types::Range::create(r)}};
    }

    ~FileInputHD() throw() {}

    Constant
    get(const Context& index) const
    {
      //bestfitting will guarantee that the index is valid
      uint64_t i = get_constant_pointer<mpz_class>
        (index.lookup(DIM_ZERO)).get_ui();

      m_stream.seekg(i);

      char c;
      m_stream.get(c);

      //return it as an integer for now
      return Types::Intmp::create(static_cast<uint8_t>(c));
    }

    Tuple
    variance() const
    {
      return m_variance;
    }

    private:
    mutable std::ifstream m_stream;
    Tuple m_variance;
  };
  #endif

  class FileArrayInFn : public BaseFunctionType
  {
    public:
    FileArrayInFn(System& system)
    : m_system(system)
    {
    }

    ~FileArrayInFn() throw() {}

    private:

    Constant
    applyFn(const Constant& arg) const;

    Constant
    applyFn(const std::vector<Constant>& args) const;

    BaseFunctionType*
    cloneSelf() const
    {
      return new FileArrayInFn(*this);
    }

    System& m_system;
  };

  class FileArrayOutFn : public BaseFunctionType
  {
    public:
    FileArrayOutFn(System& system)
    : m_system(system)
    {}

    ~FileArrayOutFn() throw() {}

    private:

    FileArrayOutFn*
    cloneSelf() const
    {
      return new FileArrayOutFn(*this);
    }

    Constant
    applyFn(const Constant& arg) const;

    Constant
    applyFn(const std::vector<Constant>& args) const;

    System& m_system;
  };

  class FileArrayInHD : public InputHD
  {
    public:

    FileArrayInHD(const u32string& file, System& system);

    ~FileArrayInHD() throw() {}

    Tuple
    variance() const;

    Constant
    get(const Context& k) const;

    private:
    ArrayNHD<mpz_class, 2>* m_array;
  };

  class FileArrayOutHD : public OutputHD
  {
    public:

    FileArrayOutHD
    (
      const u32string& file, 
      const mpz_class& height,
      const mpz_class& width,
      System& system
    );

    ~FileArrayOutHD() throw() {}

    Tuple
    variance() const;

    void
    commit();

    void
    put(const Context& t, const Constant& c);

    private:
    size_t m_height;
    size_t m_width;

    ArrayNHD<mpz_class, 2>* m_array;
    std::ofstream m_file;
  };
}
