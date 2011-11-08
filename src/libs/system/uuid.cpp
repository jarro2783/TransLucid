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

#include <iostream>
#include <tl/fixed_indexes.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types_util.hpp>
#include <tl/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace std
{
  size_t
  hash<boost::uuids::uuid>::
  operator()(const boost::uuids::uuid& u) const
  {
    return boost::uuids::hash_value(u);
  }
}

namespace boost
{
  namespace uuids
  {
    std::ostream&
    operator<<(std::ostream& os, const uuid& id)
    {
      for(int i : id)
      {
        os << std::hex << i;
      }
      return os;
    }
  }
}

namespace TransLucid
{
  namespace
  {
    boost::uuids::basic_random_generator<boost::mt19937> uuid_generator;

    TypeFunctions uuid_type_functions =
      {
        &Types::UUID::equality,
        &Types::UUID::hash,
        &delete_ptr<uuid>
      };
  }

  uuid
  generate_uuid()
  {
    return uuid_generator();
  }

  uuid
  generate_nil_uuid()
  {
    return boost::uuids::nil_generator()();
  }

  namespace Types
  {
    namespace UUID
    {
      Constant
      create(const uuid& i)
      {
        return make_constant_pointer
          (i, &uuid_type_functions, TYPE_INDEX_UUID);
      }

      const uuid&
      get(const Constant& u)
      {
        return get_constant_pointer<uuid>(u);
      }

      bool 
      equality(const Constant& lhs, const Constant& rhs)
      {
        return get(lhs) == get(rhs);
      }

      size_t
      hash(const Constant& c)
      {
        return std::hash<uuid>()(get(c));
      }
    }
  }
}
