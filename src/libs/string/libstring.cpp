#include "libstring.hpp"
#include <tl/fixed_indexes.hpp>
#include <tl/equation.hpp>
#include <tl/utility.hpp>
#include <tl/system.hpp>

namespace TransLucid
{

namespace LibString
{

HD*
create_plus()
{
}

void
register_one_op(HD* hd, SystemHD& i)
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

  tuple_t context =
  {
    {
      DIM_VALID_GUARD,
      Constant(Guard(GuardHD(Tuple(guard))),
                     TYPE_INDEX_GUARD)
    },
    {
      DIM_ID,
      generate_string(U"OP")
    }
  };

  i.addExpr(Tuple(context), hd);
}

void
register_string_ops(SystemHD& i)
{
  register_one_op(create_plus(), i);
}

}

}

extern "C"
{

void
lib_string_init(TransLucid::SystemHD& i)
{
  TransLucid::LibString::register_string_ops(i);
}

} //extern "C"
