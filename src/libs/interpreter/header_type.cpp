#if 0

#include <tl/header_type.hpp>
#include <tl/parser.hpp>

namespace TransLucid
{

size_t
HeaderType::hash() const
{
   return 0;
}

void
HeaderType::parseString
(
  const ustring_t& s,
  const Tuple& c,
  Interpreter& i
)
{
  #if 0
  Parser::HeaderGrammar hg(m_header, i.parsers());

  Parser::UIterator iter(s);

  Parser::iterator_t begin(Parser::Iterator(iter),
                           Parser::Iterator(iter.make_end()));

  i.parseRange(begin, Parser::iterator_t(), hg);
  #endif
}

void
HeaderType::parseFile
(
  const ustring_t& file,
  const Tuple& c,
  Interpreter& i
)
{
}

bool
HeaderType::operator==(const HeaderType& rhs) const
{
  return m_header == rhs.m_header;
}

}

#endif
