/* UUID definitions.
   Copyright (C) 2011, 2012 Jarryd Beck

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

#ifndef TL_UUID_HPP_INCLUDED
#define TL_UUID_HPP_INCLUDED

#include <tl/detail/types_detail.hpp>

namespace TransLucid
{
  class uuid
  {
    static const size_t static_size = 16;
    public:
    typedef uint8_t* iterator;
    typedef const uint8_t* const_iterator;

    iterator begin() { return data; }
    iterator end() { return data + static_size; }
    const_iterator begin() const { return data; }
    const_iterator end() const { return data + static_size; }

    constexpr size_t size() { return static_size; }

    private:
    uint8_t data[static_size];
  };

  inline bool operator==(const uuid& lhs, const uuid& rhs)
  {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  inline bool operator<(const uuid& lhs, const uuid& rhs)
  {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), 
      rhs.end());
  }

  inline bool operator!=(const uuid& lhs, const uuid& rhs)
  {
    return !(lhs == rhs);
  }

  inline std::size_t hash_value(uuid const& u) /* throw() */
  {
    std::size_t seed = 0;
    for(uuid::const_iterator i=u.begin(); i != u.end(); ++i)
    {
      seed ^= static_cast<std::size_t>(*i) + 
        0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
  }

  struct InvalidUUID
  {
  };

  uuid
  parse_uuid(const u32string& text);

  uuid
  generate_uuid();

  uuid
  generate_nil_uuid();

  namespace detail
  {
    template <>
    struct clone<uuid>
    {
      uuid*
      operator()(const uuid& u)
      {
        return new uuid(u);
      }
    };
  }
}

namespace std
{
  template <>
  struct hash<TransLucid::uuid>
  {
    public:
    size_t operator()(const TransLucid::uuid& u) const;
  };
}


#endif
