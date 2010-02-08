#include <tl/cache.hpp>
#include <tl/types.hpp>
#include <tl/builtin_types.hpp>
#include <tl/interpreter.hpp>

namespace TransLucid
{

std::pair<bool, TypedValue>
LazyWarehouse::lookupCalc(const ustring_t& name, const Tuple& c)
{
  #if 0
  CacheMapping::iterator iter = m_cache.find(name);
  if (iter == m_cache.end())
  {
    //add calc because it wasn't found
    TupleToValue m;
    m.insert(std::make_pair
      (c, TypedValue(ValueCalc(), m_interpreter.typeRegistry().indexCalc())));
    m_cache.insert(std::make_pair(name, m));
  }
  else
  {
    TupleToValue& values = iter->second;
    TupleToValue::iterator titer = values.find(c);
    if (titer == values.end())
    {
      //add calc because it wasn't found
      iter->second.insert
        (std::make_pair
          (c, TypedValue(ValueCalc(),
                         m_interpreter.typeRegistry().indexCalc())));
    }
    else
    {
      //return the actual value
      return std::make_pair(true, titer->second);
    }
  }
  #endif
  return std::make_pair(false, TypedValue());
}

void
LazyWarehouse::add
(const ustring_t& name, const TypedValue& value, const Tuple& c)
{
  #if 0
  CacheMapping::iterator iter = m_cache.find(name);
  if (iter == m_cache.end())
  {
    TupleToValue m;
    m.insert(std::make_pair(c, value));
    m_cache.insert(std::make_pair(name, m));
  }
  else
  {
    TupleToValue::iterator titer = iter->second.find(c);
    if (titer == iter->second.end())
    {
      iter->second.insert(std::make_pair(c, value));
    }
    else
    {
      titer->second = value;
    }
  }
  #endif
}

}
