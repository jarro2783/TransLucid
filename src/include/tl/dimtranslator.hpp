/* Maps dimensions to indexes.
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

#ifndef DIMTRANSLATOR_HPP_INCLUDED
#define DIMTRANSLATOR_HPP_INCLUDED

#include <tl/types.hpp>
#include <boost/unordered_map.hpp>

namespace TransLucid
{
  /**
   * @brief Stores dimensions mapping to integers.
   *
   * Maps typed values and dimensions to integers as a speed
   * optimisation.
   **/
  class DimensionTranslator
  {
    public:

    DimensionTranslator();

    /**
     * @brief Retrieves the value of a named dimension.
     **/
    size_t
    lookup(const u32string& name);

    /**
     * @brief Retrieves the value of a typed value dimension.
     **/
    size_t
    lookup(const TypedValue& value);

    private:

    size_t m_nextIndex;

    typedef boost::unordered_map<u32string, size_t> ustring_size_map;
    typedef boost::unordered_map<TypedValue, size_t> ustring_type_map;

    ustring_size_map m_namedDims;
    ustring_type_map m_typedDims;
  };
}

#endif // DIMTRANSLATOR_HPP_INCLUDED
