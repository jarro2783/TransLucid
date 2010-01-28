#ifndef SET_TYPES_HPP_INCLUDED
#define SET_TYPES_HPP_INCLUDED

#include <tl/builtin_types.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid {
   class TypeAsSet : public SetBase {
      public:
      TypeAsSet(size_t index);

      private:
      size_t m_index;
   };
}

#endif // SET_TYPES_HPP_INCLUDED
