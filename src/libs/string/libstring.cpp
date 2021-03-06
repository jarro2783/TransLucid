#include "libstring.hpp"
#include <tl/fixed_indexes.hpp>
#include <tl/equation.hpp>
#include <tl/utility.hpp>
#include <tl/system.hpp>
#include <tl/ophd.hpp>

/**
 * @file libstring.cpp
 * The string library implementation.
 */

namespace TransLucid
{

/**
 * The libstring namespace.
 * Everything that libstring does goes in here.
 */
namespace LibString
{

/**
 * The string argument type.
 */
template <int index>
struct StringArgType
{
  /**
   * The type of the string arg is always String.
   */
  typedef String type;
};

/**
 * Concatenates strings.
 * A functor which is used with TransLucid::OpWS to concatenate strings.
 */
struct StringAdder
{
  /**
   * The actual string operation.
   */
  TaggedConstant
  operator()(const String& lhs, const String& rhs, const Tuple& c) const
  {
    return TaggedConstant
    (
      Constant(String(lhs.value() + rhs.value()), TYPE_INDEX_USTRING),
      c
    );
  }
};

/**
 * Create a string concatenator.
 * A generator function which creates a hyperdaton which concatenates two
 * strings.
 * @param i The current system.
 * @return A hyperdaton which takes two strings in dimensions arg0 and arg1
 * and concatenates them.
 */
WS*
create_plus(System& i)
{
  return new OpWS<StringAdder, 
    StringArgType, TYPE_INDEX_USTRING, TYPE_INDEX_USTRING>(i);
}

/**
 * Register an operation with the system. Registers any operation over
 * String * String with the system @a i which is defined by the hyperdaton
 * @a hd.
 * @param hd The operation hyperdaton.
 * @param i The system hyperdaton.
 */
void
register_one_op(WS* hd, System& i)
{
  tuple_t guard =
  {
    {
      get_dimension_index(&i, U"arg0"),
      Constant(Type(TYPE_INDEX_USTRING), TYPE_INDEX_TYPE)
    },
    {
      get_dimension_index(&i, U"arg1"),
      Constant(Type(TYPE_INDEX_USTRING), TYPE_INDEX_TYPE)
    },
    {
      DIM_NAME,
      generate_string(U"operator+")
    }
  };

  i.addEquation(U"OP", GuardWS(Tuple(guard)), hd);
}

/**
 * Register all of the string operations.
 * @param i The current system hyperdaton.
 */
void
register_string_ops(System& i)
{
  register_one_op(create_plus(i), i);
}

}

}

extern "C"
{

/**
 * Initialise the string library.
 * @param i The current system hyperdaton.
 */
void
lib_string_init(TransLucid::System& i)
{
  TransLucid::LibString::register_string_ops(i);
}

} //extern "C"
