/* TODO: Give a descriptor.
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

#include <tl/range.hpp>

namespace TransLucid
{

Range::Range()
: m_lower(0), m_upper(0)
{
}

Range::Range(const mpz_class* lower, const mpz_class* upper)
: m_lower(lower != 0 ? new mpz_class(*lower) : 0),
m_upper(upper != 0 ? new mpz_class(*upper) : 0)
{
}

Range::Range(const Range& other)
: m_lower(other.m_lower != 0 ? new mpz_class(*other.m_lower) : 0),
m_upper(other.m_upper != 0 ? new mpz_class(*other.m_upper) : 0)
{
  mpz_class* lower = 0;
  mpz_class* upper = 0;
  try
  {
    lower = other.m_lower != 0 ? new mpz_class(*other.m_lower) : 0;
    upper = other.m_upper != 0 ? new mpz_class(*other.m_upper) : 0;
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

    mpz_class* lower = 0;
    mpz_class* upper = 0;

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

  if (m_lower == 0) {
    boost::hash_combine(seed, 0);
  }
  else
  {
    boost::hash_combine(seed, *m_lower);
  }

  if (m_upper == 0)
  {
    boost::hash_combine(seed, 0);
  }
  else
  {
    boost::hash_combine(seed, *m_upper);
  }

  return seed;
}

void
Range::print(std::ostream& os, const Tuple& context) const
{
  if (m_lower == 0)
  {
    os << "inf";
  }
  else
  {
    os << *m_lower;
  }

  os << " .. ";

  if (m_upper == 0)
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
  ((m_lower == 0 && rhs.m_lower == 0)
     || (m_lower != 0 && rhs.m_lower != 0 && *m_lower == *rhs.m_lower))
  &&
  ((m_upper == 0 && rhs.m_upper == 0)
     || (m_upper != 0 && rhs.m_upper != 0 && *m_upper == *rhs.m_upper))
  ;
}

bool
Range::within(const Intmp& value) const
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

  return (!m_lower || *m_lower <= value.value())
      && (!m_upper || *m_upper >= value.value());
}

bool
Range::within(const Range& other) const
{
  //is the other range inside this range
  //lower must be <= other.lower and upper must be >= other.upper
  return (m_lower == 0
  || (other.m_lower != 0 &&  *m_lower <= *other.m_lower))
  &&
  (m_upper == 0
  || (other.m_upper != 0 && *m_upper >= *other.m_upper));
}

}
