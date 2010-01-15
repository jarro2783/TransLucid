#ifndef UTILITY_HPP_INCLUDED
#define UTILITY_HPP_INCLUDED

#include <tl/types.hpp>
#include <vector>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/classification.hpp>

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
}

#endif // UTILITY_HPP_INCLUDED
