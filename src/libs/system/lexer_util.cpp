#include <tl/lexer_util.hpp>

namespace TransLucid { namespace Parser { namespace detail {

template <typename Iterator>
mpq_class
init_mpq(const Iterator& begin, const Iterator& end, int base)
{
  std::string s;
  s.reserve(end - begin);

  Iterator current = begin;
  while (current != end)
  {
    if (*current == '_')
    {
      s += '/';
    }
    else
    {
      s += *current;
    }
    ++current;
  }

  mpq_class value(s, base);

  return value;
}

} //namespace detail

mpq_class
init_mpq
(
  const std::wstring::const_iterator& begin,
  const std::wstring::const_iterator& end,
  int base
)
{
  return detail::init_mpq(begin, end, base);
}

} }
