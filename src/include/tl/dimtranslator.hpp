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

#include <tl/object_registry.hpp>
#include <tl/types.hpp>
#include <unordered_map>

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
    lookup(const Constant& value);

    /**
     * @brief Returns the value of a unique dimension.
     * The next index counter is incremented, and it isn't mapped to a name,
     * therefore, it is unique and hidden.
     */
    size_t
    unique()
    {
      return m_nextIndex++;
    }

    private:

    size_t m_nextIndex;

    ObjectRegistry<u32string, decltype(m_nextIndex)> m_named;
    ObjectRegistry<Constant, decltype(m_nextIndex)> m_constants;

    typedef std::unordered_map<u32string, size_t> ustring_size_map;
    typedef std::unordered_map<Constant, size_t> ustring_type_map;

    ustring_size_map m_namedDims;
    ustring_type_map m_typedDims;
  };
}

#endif // DIMTRANSLATOR_HPP_INCLUDED
