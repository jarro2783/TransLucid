#include <tl/set_types.hpp>

namespace TransLucid
{

TypeAsSet::TypeAsSet(size_t index)
: m_index(index)
{
}

bool
TypeAsSet::is_member(const TypedValue& v)
{
  return v.index() == m_index;
}

//is s a subset of this
bool
TypeAsSet::is_subset(const SetBase& s)
{
}

}
