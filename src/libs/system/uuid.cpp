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

#include <random>

#include <tl/fixed_indexes.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types_util.hpp>
#include <tl/uuid.hpp>

namespace TransLucid
{
  namespace
  {
    class RandomIterator
    {
      public:
      RandomIterator(std::random_device* rand)
      : m_rand(rand), m_count(0)
      {}

      RandomIterator(int count)
      : m_rand(nullptr), m_count(count)
      {}

      private:
      std::random_device* m_rand;

      public:
      decltype((*m_rand)()) operator*()
      {
        return (*m_rand)();
      }

      RandomIterator&
      operator++()
      { 
        ++m_count;
        return *this; 
      }

      bool
      operator==(const RandomIterator& rhs) const
      {
        return m_count == rhs.m_count;
      }

      bool
      operator!=(const RandomIterator& rhs) const
      {
        return !(*this == rhs);
      }

      private:
      int m_count;
    };

    TypeFunctions uuid_type_functions =
      {
        &Types::UUID::equality,
        &Types::UUID::hash,
        &delete_ptr<uuid>
      };

    template <typename Generator>
    void
    seed(Generator* generator)
    {
      std::random_device rand("/dev/urandom");
      RandomIterator begin(&rand);
      RandomIterator end(101);

      std::seed_seq seq(begin, end);

      generator->seed(seq);
    }

    template <class Generator>
    class uuid_random_generator
    {
      typedef typename Generator::result_type unit_type; 
      typedef std::independent_bits_engine
      <
        Generator,
        sizeof(unit_type)*8,
        unit_type
      >full_bits_generator;

      public:
      uuid_random_generator()
      {
        seed(&generator);
      }

      uuid
      operator()()
      {
        uuid u;

        int i = 0;
        unit_type random = generator();
        for (auto it = u.begin(); it != u.end(); ++it, ++i)
        {
          if (i == sizeof(unit_type))
          {
            random = generator();
            i = 0;
          }

          *it = (random >> (i*8)) & 0xFF;
        }

        return u;
      }

      private:
      full_bits_generator generator;
    };

    uuid_random_generator<std::mt19937> uuid_generator;
  }

  uuid
  generate_uuid()
  {
    return uuid_generator();
  }

  uuid
  generate_nil_uuid()
  {
    uuid u;
    std::fill(u.begin(), u.end(), 0);

    return u;
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

namespace std
{
  size_t hash<TransLucid::uuid>::operator()(const TransLucid::uuid& u) const
  {
    return TransLucid::hash_value(u);
  }
}

