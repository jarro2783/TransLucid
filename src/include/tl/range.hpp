/* Range of values.
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

#ifndef RANGE_HPP_INCLUDED
#define RANGE_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/builtin_types.hpp>
#include <gmpxx.h>

namespace TransLucid
{
  class Range : public TypedValue
  {
    public:
    Range();
    Range(const mpz_class* lower, const mpz_class* upper);
    Range(const Range& other);
    Range&
    operator=(const Range& rhs);

    ~Range();

    void
    print(std::ostream& os, const Tuple& context) const;

    size_t
    hash() const;

    bool
    operator==(const Range& rhs) const;

    bool
    within(const Intmp& value) const;

    bool
    within(const Range& other) const;

    bool
    operator<(const Range& rhs) const
    {
      if (m_lower == 0 && rhs.m_lower != 0)
      {
        return true;
      }

      if (m_lower != 0 && rhs.m_lower == 0)
      {
        return false;
      }

      if (m_lower && rhs.m_lower)
      {
        if (*m_lower != *rhs.m_lower)
        {
          return *m_lower < *rhs.m_lower;
        }
      }

      //the two lower bounds must be equal
      if (m_upper == 0 && rhs.m_upper != 0)
      {
        return false;
      }

      if (m_upper != 0 && rhs.m_upper == 0)
      {
        return true;
      }

      if (m_upper && rhs.m_upper)
      {
        return *m_upper < *rhs.m_upper;
      }
      //everything is equal if we got here
      return false;
    }

    private:
    const mpz_class* m_lower;
    const mpz_class* m_upper;
  };
}

#endif // RANGE_HPP_INCLUDED
