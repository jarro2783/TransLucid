#include "libstring.hpp"

namespace TransLucid
{

namespace LibString
{

void
register_string_ops(TransLucid::SystemHD& i)
{
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
