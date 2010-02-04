#ifndef CONSTHD_HPP_INCLUDED
#define CONSTHD_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid {
   namespace ConstHD {

      class ConstantHD : public HD {
         public:
         ConstantHD(HD *system)
         : m_system(system)
         {}

         virtual void addExpr(const Tuple& k, HD *h) {
         }

         protected:
         HD *m_system;
      };

      class UChar : public ConstantHD {
         public:
         static const char *name;

         UChar(HD *system)
         : ConstantHD(system)
         {}

         TaggedValue operator()(const Tuple& k);
      };

      class Intmp : public ConstantHD {
         public:
         static const char *name;

         Intmp(HD *system)
         : ConstantHD(system)
         {}

         TaggedValue operator()(const Tuple& k);

         private:
      };

      class UString : public ConstantHD {
         public:
         static const char *name;

         UString(HD *system)
         : ConstantHD(system)
         {}

         TaggedValue operator()(const Tuple& k);
      };

      //this is not the intmp builder, this returns a constant intmp
      class IntmpConst : public HD {
         public:
         IntmpConst(const mpz_class& v)
         : m_v(v)
         {}

         TaggedValue operator()(const Tuple& k) {
            return TaggedValue(TypedValue(TransLucid::Intmp(m_v), TYPE_INDEX_INTMP), k);
         }

         void addExpr(const Tuple& k, HD *h) {
         }

         private:
         mpz_class m_v;
      };

      class TypeConst : public ConstantHD {
         public:

         TypeConst(size_t index)
         : ConstantHD(0), m_index(index)
         {}

         TaggedValue operator()(const Tuple& k) {
            return TaggedValue(TypedValue(TypeType(m_index), TYPE_INDEX_TYPE), k);
         }

         private:
         size_t m_index;
      };
   } //namespace ConstHD
} //namespace TransLucid

#endif // CONSTHD_HPP_INCLUDED
