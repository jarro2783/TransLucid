/* Predefined type and dimension indexes which must exist
   for the system to work.
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

#ifndef FIXED_INDEXES_HPP_INCLUDED
#define FIXED_INDEXES_HPP_INCLUDED

namespace TransLucid
{

  //predefined unique indexes
  enum TypeIndexes
  {
    //types
    TYPE_INDEX_BOOL,
    TYPE_INDEX_SPECIAL,
    TYPE_INDEX_INTMP,
    TYPE_INDEX_UCHAR,
    TYPE_INDEX_USTRING,
    TYPE_INDEX_DIMENSION,
    TYPE_INDEX_TUPLE,
    TYPE_INDEX_TYPE,
    TYPE_INDEX_GUARD,
    TYPE_INDEX_RANGE,

    //the last one
    TYPE_INDEX_LAST
  };

  enum DimensionIndexes
  {
    //dimensions
    DIM_ID,
    DIM_TYPE,
    DIM_TEXT,
    DIM_NAME,
    DIM_VALUE,
    DIM_TIME,
    DIM_VALID_GUARD,

    //the last one
    DIM_INDEX_LAST
  };

} //namespace TransLucid

#endif // FIXED_INDEXES_HPP_INCLUDED
