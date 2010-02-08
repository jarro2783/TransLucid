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

    virtual void
    addExpr(const Tuple& k, HD* h) = 0;
  };
} //namespace TransLucid

#endif // HYPERDATON_HPP_INCLUDED
