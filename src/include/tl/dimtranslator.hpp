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

#include <unordered_map>

#include <tl/object_registry.hpp>
#include <tl/types.hpp>

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
    dimension_index
    lookup(const u32string& name);

    /**
     * @brief Retrieves the value of a typed value dimension.
     **/
    dimension_index
    lookup(const Constant& value);

    /**
     * @brief Returns the value of a unique dimension.
     * The next index counter is incremented, and it isn't mapped to a name,
     * therefore, it is unique and hidden.
     */
    dimension_index
    unique()
    {
      return m_nextIndex++;
    }

    private:

    dimension_index m_nextIndex;

    ObjectRegistry<u32string, decltype(m_nextIndex)> m_named;
    ObjectRegistry<Constant, decltype(m_nextIndex)> m_constants;
  };
}

#endif // DIMTRANSLATOR_HPP_INCLUDED
