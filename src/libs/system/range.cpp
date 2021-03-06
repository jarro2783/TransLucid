/* Range type.
   Copyright (C) 2009--2012 Jarryd Beck

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

/** @file range.cpp
 * The range object. Defines a set of integers in a range.
 */

#include <functional>

#include <gmpxx.h>

#include <tl/output.hpp>
#include <tl/range.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

Range::Range()
: m_lower(nullptr), m_upper(nullptr)
{
}

Range::Range(const mpz_class* lower, const mpz_class* upper)
: m_lower(lower != nullptr ? new mpz_class(*lower) : nullptr),
m_upper(upper != nullptr ? new mpz_class(*upper) : nullptr)
{
}

Range::Range(const Range& other)
: m_lower(nullptr), m_upper(nullptr)
{
  mpz_class* lower = nullptr;
  mpz_class* upper = nullptr;
  try
  {
    lower = other.m_lower != nullptr ? new mpz_class(*other.m_lower) : nullptr;
    upper = other.m_upper != nullptr ? new mpz_class(*other.m_upper) : nullptr;
    m_lower = lower;
    m_upper = upper;
  }
  catch (...)
  {
    delete lower;
    delete upper;
    throw;
  }
}

Range&
Range::operator=(const Range& rhs)
{
  if (this != &rhs)
  {
    delete m_lower;
    delete m_upper;

    mpz_class* lower = nullptr;
    mpz_class* upper = nullptr;

    try
    {
      if (rhs.m_lower)
      {
        lower = new mpz_class(*rhs.m_lower);
      }
      if (rhs.m_upper)
      {
        upper = new mpz_class(*rhs.m_upper);
      }
      m_lower = lower;
      m_upper = upper;
    }
    catch (...)
    {
      delete lower;
      delete upper;
      throw;
    }
  }

  return *this;
}

Range::~Range()
{
  delete m_lower;
  delete m_upper;
}

size_t
Range::hash() const
{
  size_t seed = 0;

  if (m_lower == nullptr) {
    hash_combine_hasher(0, seed);
  }
  else
  {
    hash_combine_hasher(*m_lower, seed);
  }

  if (m_upper == nullptr)
  {
    hash_combine_hasher(0, seed);
  }
  else
  {
    hash_combine_hasher(*m_upper, seed);
  }

  return seed;
}

void
Range::print(std::ostream& os) const
{
  if (m_lower == nullptr)
  {
    os << "inf";
  }
  else
  {
    os << *m_lower;
  }

  os << " .. ";

  if (m_upper == nullptr)
  {
    os << "inf";
  }
  else
  {
    os << *m_upper;
  }
}

bool
Range::operator==(const Range& rhs) const
{
  //for both upper and lower
  //either both rhs and lhs are 0, or they are not zero and are equal
  return
  ((m_lower == nullptr && rhs.m_lower == nullptr)
     || (m_lower != nullptr && rhs.m_lower != nullptr && 
         *m_lower == *rhs.m_lower))
  &&
  ((m_upper == nullptr && rhs.m_upper == nullptr)
     || (m_upper != nullptr && rhs.m_upper != nullptr && 
         *m_upper == *rhs.m_upper))
  ;
}

bool
Range::operator<(const Range& rhs) const
{
  if (m_lower == nullptr && rhs.m_lower != nullptr)
  {
    return true;
  }

  if (m_lower != nullptr && rhs.m_lower == nullptr)
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
  if (m_upper == nullptr && rhs.m_upper != nullptr)
  {
    return false;
  }

  if (m_upper != nullptr && rhs.m_upper == nullptr)
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

bool
Range::within(const mpz_class& value) const
{

  #if 0
  std::cout << "Range::within" << std::endl;
  std::cout << value.value() << std::endl;

  if (m_lower)
  {
    std::cout << *m_lower;
  }
  else
  {
    std::cout << "inf";
  }

  std::cout << " .. ";

  if (m_upper)
  {
    std::cout << *m_upper;
  }
  else
  {
    std::cout << "inf";
  }

  std::cout << std::endl;

  bool within =
     (!m_lower || *m_lower <= value.value())
  && (!m_upper || *m_upper >= value.value());

  std::cout << "within: ";

  if (within)
  {
    std::cout << "true";
  }
  else
  {
    std::cout << "false";
  }
  std::cout << std::endl;
  #endif

  return (!m_lower || *m_lower <= value)
      && (!m_upper || *m_upper >= value);
}

bool
Range::within(const Range& other) const
{
  //is the other range inside this range
  //lower must be <= other.lower and upper must be >= other.upper
  return (m_lower == nullptr
  || (other.m_lower != nullptr &&  *m_lower <= *other.m_lower))
  &&
  (m_upper == nullptr
  || (other.m_upper != nullptr && *m_upper >= *other.m_upper));
}

bool
Range::overlaps(const Range& other) const
{
  return ((!m_upper || (!other.m_lower || *m_upper > *other.m_lower))
      &&
      (!other.m_upper || (!m_lower || *other.m_upper > *m_lower))
  );
}

Range
Range::join(const Range& rhs) const
{
  const mpz_class* lower = nullptr;
  const mpz_class* upper = nullptr;

  if (m_lower && rhs.m_lower)
  {
    if (*m_lower < *rhs.m_lower)
    {
      lower = m_lower;
    }
    else
    {
      lower = rhs.m_lower;
    }
  }

  if (m_upper && rhs.m_upper)
  {
    if (*m_upper > *rhs.m_upper)
    {
      upper = m_upper;
    }
    else
    {
      upper = rhs.m_upper;
    }
  }

  return Range(lower, upper);
}

}
