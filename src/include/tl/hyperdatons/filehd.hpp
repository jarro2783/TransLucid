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

#include <tl/charset.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/types_util.hpp>

#include <fstream>

namespace TransLucid
{
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
      mpz_class right(9223372036854775807);
      Range r(&left, &right); 

      m_variance = tuple_t{{DIM_ZERO,Types::Range::create(r)}};
    }

    ~FileInputHD() throw() {}

    Constant
    get(const Tuple& index) const
    {
      //bestfitting will guarantee that the index is valid
      uint64_t i = get_constant_pointer<mpz_class>
        (index.find(DIM_ZERO)->second).get_ui();

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

  class FileOutputHD : public OutputHD
  {
  };

  class FileIOHD : public IOHD
  {
  };
}
