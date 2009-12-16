#ifndef CACHE_HPP_INCLUDED
#define CACHE_HPP_INCLUDED

#include <map>
#include <tl/types.hpp>

namespace TransLucid {

   class Interpreter;

   class LazyWarehouse {
      public:

      LazyWarehouse(Interpreter& i)
      : m_interpreter(i)
      {}

      //looks up and if not found adds a calc entry
      std::pair<bool, TypedValue> lookupCalc(const ustring_t& name, const Tuple& c);

      void add(const ustring_t& name, const TypedValue& value, const Tuple& c);

      private:
      typedef std::map<Tuple, TypedValue> TupleToValue;
      typedef std::map<ustring_t, TupleToValue> CacheMapping;
      CacheMapping m_cache;
      Interpreter& m_interpreter;
   };

}

#endif // CACHE_HPP_INCLUDED
