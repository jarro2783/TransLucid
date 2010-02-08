#ifndef SET_TYPES_HPP_INCLUDED
#define SET_TYPES_HPP_INCLUDED

#include <tl/builtin_types.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  class TypeAsSet : public SetBase
  {
    public:
    TypeAsSet(size_t index);

    bool
    is_member(const TypedValue& v);

    //is s a subset of this
    bool
    is_subset(const SetBase& s);

    private:
    size_t m_index;
  };
}

#endif // SET_TYPES_HPP_INCLUDED
