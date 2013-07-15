/* Predefined type and dimension indexes which must exist
   for the system to work.
   Copyright (C) 2011, 2012 Jarryd Beck

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

/**
 * @file fixed_indexes.hpp
 * Defines all the builtin index values.
 */

#ifndef FIXED_INDEXES_HPP_INCLUDED
#define FIXED_INDEXES_HPP_INCLUDED

#include <utility>

namespace TransLucid
{

  //predefined unique indexes
  //IMPORTANT: if you change these, then you must also
  //change the bestfitting function table TypeComparators in bestfit.cpp
  //and the type strings in types.cpp
  enum TypeIndexes
  {
    //types
    TYPE_INDEX_ERROR = 0,
    TYPE_INDEX_BOOL,
    TYPE_INDEX_SPECIAL,
    TYPE_INDEX_INTMP,
    TYPE_INDEX_UCHAR,
    TYPE_INDEX_USTRING,
    TYPE_INDEX_FLOATMP,
    TYPE_INDEX_DIMENSION,
    TYPE_INDEX_REGION,
    TYPE_INDEX_TUPLE,
    TYPE_INDEX_TYPE, //10
    TYPE_INDEX_RANGE,
    TYPE_INDEX_UNION,
    TYPE_INDEX_BASE_FUNCTION,
    TYPE_INDEX_VALUE_FUNCTION,
    TYPE_INDEX_NAME_FUNCTION,
    TYPE_INDEX_UUID,
    TYPE_INDEX_INTENSION,
    TYPE_INDEX_INHD,
    TYPE_INDEX_IOHD,
    TYPE_INDEX_OUTHD, //20
    TYPE_INDEX_CALC,
    TYPE_INDEX_DEMAND,

    //the last one
    TYPE_INDEX_LAST
  };

  //this is defined in types.cpp and must be changed if the set of
  //fixed types are changed
  extern char32_t const* const type_index_names[];

  inline
  std::pair<char32_t const*, TypeIndexes>
  type_name_pair(TypeIndexes t)
  {
    return std::make_pair(type_index_names[t], t);
  }

  //if you change something here, make sure to update the names in
  //src/libs/system/dimtranslator.cpp
  enum DimensionIndexes
  {
    //dimensions
    DIM_TIME = -1,
    DIM_PRIORITY = -2,
    DIM_ALL = -3,
    DIM_ID = -4,
    DIM_TYPE = -5,
    DIM_TEXT = -6,
    DIM_NAME = -7,
    DIM_VALUE = -8,
    DIM_SYMBOL = -9,

    //the two global dims for phi functions
    DIM_PI = -10,
    DIM_PSI = -11,

    //for array indices, these are the integers 0, 1, 2
    DIM_ZERO = -12,
    DIM_ONE = -13,
    DIM_TWO = -14,

    //the constructor dim for data types
    DIM_CONS = -15,

    //the rho dimension for the call stack
    DIM_RHO = -16,

    //fix some defaults for arg0, arg1, arg2 and arg3 because they are commonly
    //used in code
    DIM_ARG0 = -17,
    DIM_ARG1 = -18,
    DIM_ARG2 = -19,
    DIM_ARG3 = -20,

    //the last one
    DIM_INDEX_LAST = -21
  };

} //namespace TransLucid

#endif // FIXED_INDEXES_HPP_INCLUDED
