#ifndef TL_TYPES_BASIC_HPP_INCLUDED
#define TL_TYPES_BASIC_HPP_INCLUDED

#include <string>

namespace TransLucid
{
  typedef uint16_t type_index;
  typedef std::u32string u32string;
  typedef int32_t dimension_index;

  enum Special
  {
    SP_ERROR, /**<Error value. Should never have this value, having a special
    of this value means an error occured somewhere.*/
    SP_ACCESS, /**<Access error. Something requested could not be accessed.*/
    SP_TYPEERROR,
    SP_DIMENSION,
    SP_ARITH,
    SP_UNDEF,
    SP_CONST,
    SP_MULTIDEF,
    SP_LOOP,
    SPECIAL_LAST //the number of specials, not an actual special value
  };
}

namespace std
{
  template <>
  struct hash<TransLucid::Special>
  {
    size_t
    operator()(TransLucid::Special s) const;
  };
}

#endif
