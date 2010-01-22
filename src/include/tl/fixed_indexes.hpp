#ifndef FIXED_INDEXES_HPP_INCLUDED
#define FIXED_INDEXES_HPP_INCLUDED

namespace TransLucid {

   //predefined unique indexes
   enum {
      //types
      TYPE_INDEX_USTRING,
      TYPE_INDEX_INTMP,
      TYPE_INDEX_BOOL,

      //dimensions
      DIM_TYPE,
      DIM_TEXT,
      DIM_NAME,
      DIM_ID,

      //the last one
      RESERVED_INDEX_LAST

   };

} //namespace TransLucid

#endif // FIXED_INDEXES_HPP_INCLUDED
