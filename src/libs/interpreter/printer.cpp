#include <tl/printer.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{

TaggedValue
ValuePrinter::operator()(const Tuple& k)
{
  u32string s = U"<";

  Tuple::const_iterator iter = k.find(DIM_VALUE);

  if (iter == k.end())
  {
  }
}

void
ValuePrinter::addExpr(const Tuple& k, HD* h)
{
}

}
