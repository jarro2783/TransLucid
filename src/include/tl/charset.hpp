#ifndef CHARSET_HPP_INCLUDED
#define CHARSET_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{
  std::string
  utf32_to_utf8(const u32string& s);

  std::string
  u32_to_ascii(const u32string& s);
}

namespace std
{
  inline ostream&
  operator<<(ostream& os, const u32string& s)
  {
    os << TransLucid::utf32_to_utf8(s);
    return os;
  }

#if 0
  inline istream&
  operator>>(istream& is, u32string& s)
  {

  }
#endif
}

#endif // CHARSET_HPP_INCLUDED
