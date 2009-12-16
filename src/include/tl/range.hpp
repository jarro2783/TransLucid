#ifndef RANGE_HPP_INCLUDED
#define RANGE_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/builtin_types.hpp>
#include <gmpxx.h>

namespace TransLucid {
   class Range : public TypedValueBase {
      public:
      Range();
      Range(const mpz_class *lower, const mpz_class *upper);
      Range(const Range& other);
      Range& operator=(const Range& rhs);

      ~Range();

      static Range parse(const ustring_t& text, const Tuple& context, Interpreter& i);

      void print(std::ostream& os, const Tuple& context) const;
      size_t hash() const;

      bool operator==(const Range& rhs) const;

      bool within(const Intmp& value) const;
      bool within(const Range& other) const;

      bool operator<(const Range& rhs) const {
         if (m_lower == 0 && rhs.m_lower != 0) {
            return true;
         }

         if (m_lower != 0 && rhs.m_lower == 0) {
            return false;
         }

         if (m_lower && rhs.m_lower) {
            if (*m_lower != *rhs.m_lower) {
               return *m_lower < *rhs.m_lower;
            }
         }

         //the two lower bounds must be equal
         if (m_upper == 0 && rhs.m_upper != 0) {
            return false;
         }

         if (m_upper != 0 && rhs.m_upper == 0) {
            return true;
         }

         if (m_upper && rhs.m_upper) {
            return *m_upper < *rhs.m_upper;
         }
         //everything is equal if we got here
         return false;
      }

      private:
      const mpz_class *m_lower;
      const mpz_class *m_upper;
   };
}

#endif // RANGE_HPP_INCLUDED
