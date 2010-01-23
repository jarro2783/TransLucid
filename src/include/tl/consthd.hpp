#ifndef CONSTHD_HPP_INCLUDED
#define CONSTHD_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid {
   namespace ConstHD {
      class UChar : public HD {
         public:
         static const char *name;

         UChar(Interpreter& i)
         : m_i(i)
         {}

         TaggedValue operator()(const Tuple& k);

         void addExpr(const Tuple& k, HD *h);

         private:
         Interpreter& m_i;
      };

      //this is not the intmp builder, this returns a constant intmp
      class IntmpConst : public HD {
         public:
         IntmpConst(Interpreter& i, const mpz_class& v)
         : m_i(i), m_v(v)
         {}

         TaggedValue operator()(const Tuple& k) {
            return TaggedValue(TypedValue(Intmp(m_v), TYPE_INDEX_INTMP), k);
         }

         void addExpr(const Tuple& k, HD *h) {
         }

         private:
         Interpreter& m_i;
         mpz_class m_v;
      };
   } //namespace ConstHD
} //namespace TransLucid

#endif // CONSTHD_HPP_INCLUDED
