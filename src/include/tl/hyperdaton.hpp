#ifndef HYPERDATON_HPP_INCLUDED
#define HYPERDATON_HPP_INCLUDED

#include <tl/types.hpp>
//#include <tl/interpreter.hpp>

namespace TransLucid
{
  class HD
  {
    public:
    virtual ~HD() {}

    virtual TaggedValue
    operator()(const Tuple& k) = 0;

    virtual uuid
    addExpr(const Tuple& k, HD* h)
    {
      return nil_uuid();
    }
  };
} //namespace TransLucid

#endif // HYPERDATON_HPP_INCLUDED
