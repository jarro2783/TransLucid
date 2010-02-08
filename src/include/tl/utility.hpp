#ifndef TL_UTILITY_HPP_INCLUDED
#define TL_UTILITY_HPP_INCLUDED

#include <tl/types.hpp>
#include <vector>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/builtin_types.hpp>
#include <tl/range.hpp>

namespace TransLucid {

   class DimensionNotFound {
   };

   class SplitID {
      private:
      static const u32string m_split_match;;

      u32string::const_iterator m_begin;
      u32string::const_iterator m_end;
      const u32string& m_s;

      public:
      SplitID(const u32string& s)
      : m_s(s)
      {
         boost::iterator_range<u32string::const_iterator> r =
            boost::algorithm::find_first(s, m_split_match);
         m_begin = r.begin();
         m_end = r.end();
      }

      bool has_components() {
         return m_begin != m_end;
      }

      u32string first() {
         return u32string(m_s.begin(), m_begin);
      }

      u32string last() {
         return u32string(m_end, m_s.end());
      }
   };

   inline TypedValue generate_string(const u32string& name) {
      return TypedValue(String(name), TYPE_INDEX_USTRING);
   }

   inline mpz_class get_type_index(HD *h, const u32string& name) {
      tuple_t k;
      k[DIM_ID] = generate_string(U"TYPEINDEX"); //TypedValue(String("TYPEINDEX"), TYPE_INDEX_USTRING);
      k[DIM_TYPE] = generate_string(name); //TypedValue(String(name), TYPE_INDEX_USTRING);
      return (*h)(Tuple(k)).first.value<Intmp>().value();
   }

   inline mpz_class get_unique(HD *h) {
      tuple_t k;
      k[DIM_ID] = TypedValue(String(U"_unique"), TYPE_INDEX_USTRING);
      return (*h)(Tuple(k)).first.value<Intmp>().value();
   }

   inline size_t get_dimension_index(HD *h, const u32string& name) {
      tuple_t k;
      k[DIM_ID] = generate_string(U"DIMENSION_INDEX");
      k[DIM_TEXT] = generate_string(name);
      return (*h)(Tuple(k)).first.value<Intmp>().value().get_ui();
   }

   inline size_t get_dimension_index(HD *h, const TypedValue& v) {
      tuple_t k;
      k[DIM_ID] = generate_string(U"DIMENSION_TYPED_INDEX");
      k[DIM_VALUE] = v;
      return (*h)(Tuple(k)).first.value<Intmp>().value().get_ui();
   }

   inline TypedValue get_dimension(const Tuple& k, size_t index) {
      Tuple::const_iterator i = k.find(index);
      if (i == k.end()) {
         throw DimensionNotFound();
      }
      return i->second;
   }

   template <typename T>
   class FunctorHD : public HD {
      public:
      FunctorHD(const T& f)
      : m_f(f)
      {}

      TaggedValue operator()(const Tuple& k) {
         return m_f(k);
      }

      void addExpr(const Tuple& k, HD *h) {
      }

      private:
      T m_f;
   };

   template <typename T>
   FunctorHD<T> *generate_functor_hd(const T& f) {
      return new FunctorHD<T>(f);
   }

   template <typename T>
   u32string to_u32string(const T& s) {
      return u32string(s.begin(), s.end());
   }

   bool tupleApplicable(const Tuple& def, const Tuple& c);
   bool tupleRefines(const Tuple& a, const Tuple& b);
   bool valueRefines(const TypedValue& a, const TypedValue& b);
   bool booleanTrue(const EquationGuard& g, const Tuple& c);


}

#endif // TL_UTILITY_HPP_INCLUDED
