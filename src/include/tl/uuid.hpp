/* UUID definitions.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#define BOOST_UUID_NO_TYPE_TRAITS

#include <boost/uuid/uuid.hpp>
#include <tl/detail/types_detail.hpp>

namespace std
{
  template <>
  struct hash<boost::uuids::uuid>
  {
    size_t
    operator()(const boost::uuids::uuid& u) const;
  };
}

namespace TransLucid
{
  typedef boost::uuids::uuid uuid;

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

#endif
