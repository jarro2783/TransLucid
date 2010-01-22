#ifndef TL_UTILITY_HPP_INCLUDED
#define TL_UTILITY_HPP_INCLUDED

#include <tl/types.hpp>
#include <vector>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/builtin_types.hpp>

namespace TransLucid {

   class SplitID {
      private:
      static const ustring_t m_split_match;;

      ustring_t::const_iterator m_begin;
      ustring_t::const_iterator m_end;
      const ustring_t& m_s;

      public:
      SplitID(const ustring_t& s)
      : m_s(s)
      {
         boost::iterator_range<ustring_t::const_iterator> r =
            boost::algorithm::find_first(s, m_split_match);
         m_begin = r.begin();
         m_end = r.end();
      }

      bool has_components() {
         return m_begin != m_end;
      }

      ustring_t first() {
         return ustring_t(m_s.begin(), m_begin);
      }

      ustring_t last() {
         return ustring_t(m_end, m_s.end());
      }
   };

   inline mpz_class get_type_index(HD *h, const ustring_t& name) {
      tuple_t k;
      k[DIM_TYPE] = TypedValue(String(name), TYPE_INDEX_USTRING);
      return (*h)(Tuple(k)).first.value<Intmp>().value();
   }

   inline TypedValue generate_string(const ustring_t& name) {
      return TypedValue(String(name), TYPE_INDEX_USTRING);
   }

   inline mpz_class get_unique(HD *h) {
      tuple_t k;
      k[DIM_ID] = TypedValue(String("_unique"), TYPE_INDEX_USTRING);
      return (*h)(Tuple(k)).first.value<Intmp>().value();
   }

}

#endif // TL_UTILITY_HPP_INCLUDED
