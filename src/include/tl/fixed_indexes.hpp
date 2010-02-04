#ifndef FIXED_INDEXES_HPP_INCLUDED
#define FIXED_INDEXES_HPP_INCLUDED

namespace TransLucid {

   //predefined unique indexes
   enum Indexes {
      //types
      TYPE_INDEX_USTRING,
      TYPE_INDEX_INTMP,
      TYPE_INDEX_BOOL,
      TYPE_INDEX_SPECIAL,
      TYPE_INDEX_UCHAR,
      TYPE_INDEX_TUPLE,
      TYPE_INDEX_DIMENSION,
      TYPE_INDEX_GUARD,
      TYPE_INDEX_RANGE,
      TYPE_INDEX_PAIR,
      TYPE_INDEX_TYPE,

      //dimensions
      DIM_TYPE,
      DIM_TEXT,
      DIM_NAME,
      DIM_ID,
      DIM_VALUE,
      DIM_TIME,
      DIM_VALID_GUARD,

      //the last one
      RESERVED_INDEX_LAST
   };

} //namespace TransLucid

#endif // FIXED_INDEXES_HPP_INCLUDED
