/* Run time any dimensional array hyperdaton.
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

#include <tl/hyperdatons/arrayhd.hpp>
#include <tl/types_util.hpp>

#include <algorithm>
#include <iostream>

#include <gmpxx.h>

namespace TransLucid
{

void
ArrayHD::initialise(
  const std::vector<std::pair<dimension_index, size_t>>& bounds)
{
  size_t size = std::accumulate(bounds.begin(), bounds.end(), 1,
  [] 
  (
    size_t lhs,
    decltype(bounds.at(0))& rhs
  )
  -> size_t
  {
    return lhs * rhs.second;
  }
  );

  delete [] m_data;
  m_data = new Constant[size];

  std::cerr << "making array of size " << size << std::endl;

  m_bounds = bounds;

  //make the variance tuple
  tuple_t variance;
  mpz_class a = 0;
  for (const auto& bound : m_bounds)
  {
    mpz_class b = bound.second - 1;
    //std::cerr << "bounds are " << bound.first << ": " 
    //  << 0 << ".." << b << std::endl;
    variance.insert(std::make_pair(bound.first,
      Types::Range::create(Range(&a, &b))));
  }

  m_variance = variance;

  //set up the multipliers for indexing the array
  m_multipliers.insert(m_multipliers.end(), m_bounds.size(), 0);
  size_t prev = 1;
  auto muliter = m_multipliers.rbegin(); 
  auto bounditer = m_bounds.rbegin();
  while (muliter != m_multipliers.rend())
  {
    *muliter = prev;
    prev = prev * bounditer->second;
    ++muliter;
    ++bounditer;
  }
}

Constant
ArrayHD::get(const Context& k) const
{
  //this is the hard one
  //lookup the bounds dimensions in the context, and convert that to an
  //index

  //assume that the dimensions are correct
  auto boundsiter = m_bounds.begin();
  auto muliter = m_multipliers.begin();
  size_t index = 0;
  while (boundsiter != m_bounds.end())
  {
    auto dim = boundsiter->first;
    const auto& value = k.lookup(dim);
    mpz_class next = (get_constant_pointer<mpz_class>(value) * *muliter);
    index += next.get_ui();
    ++boundsiter;
    ++muliter;
  }

  return m_data[index];
}

void
ArrayHD::put(const Context& k, const Constant& c)
{
  //assume that the dimensions are correct
  auto boundsiter = m_bounds.begin();
  auto muliter = m_multipliers.begin();
  size_t index = 0;
  while (boundsiter != m_bounds.end())
  {
    auto dim = boundsiter->first;
    const auto& value = k.lookup(dim);
    mpz_class next = (get_constant_pointer<mpz_class>(value) * *muliter);
    index += next.get_ui();
    ++boundsiter;
    ++muliter;
  }

  std::cerr << "putting into array at index " << index << std::endl;
  m_data[index] = c;
}

void
ArrayHD::commit()
{
}

Tuple
ArrayHD::variance() const
{
  return m_variance;
}

}
