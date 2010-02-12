#ifndef CACHE_HPP_INCLUDED
#define CACHE_HPP_INCLUDED

#include <map>
#include <tl/types.hpp>

namespace TransLucid
{
  class Interpreter;

  class LazyWarehouse
  {
    public:

    LazyWarehouse(Interpreter& i)
    : m_interpreter(i)
    {}

    //looks up and if not found adds a calc entry
    std::pair<bool, TypedValue>
    lookupCalc(const u32string& name, const Tuple& c);

    void
    add(const u32string& name, const TypedValue& value, const Tuple& c);

    private:
    typedef std::map<Tuple, TypedValue> TupleToValue;
    typedef std::map<u32string, TupleToValue> CacheMapping;
    CacheMapping m_cache;
    Interpreter& m_interpreter;
  };

}

#endif // CACHE_HPP_INCLUDED
