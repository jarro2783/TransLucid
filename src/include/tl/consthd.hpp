#ifndef CONSTHD_HPP_INCLUDED
#define CONSTHD_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>
#include <tl/builtin_types.hpp>
#include <tl/fixed_indexes.hpp>

namespace TransLucid
{
  namespace ConstHD
  {

    class ExprHD : public HD
    {
      public:
      ExprHD(HD* system)
      : m_system(system)
      {}

      virtual uuid
      addExpr(const Tuple& k, HD* h)
      {
        return nil_uuid();
      }

      protected:
      HD* m_system;
    };

    class UChar : public ExprHD
    {
      public:
      static const char32_t* name;

      UChar(HD* system)
      : ExprHD(system)
      {}

      TaggedValue
      operator()(const Tuple& k);
    };

    class Intmp : public ExprHD
    {
      public:
      static const char32_t* name;

      Intmp(HD* system)
      : ExprHD(system)
      {}

      TaggedValue
      operator()(const Tuple& k);

      private:
    };

    class UString : public ExprHD
    {
      public:
      static const char32_t* name;

      UString(HD* system)
      : ExprHD(system)
      {}

      TaggedValue
      operator()(const Tuple& k);
    };

    //this is not the intmp builder, this returns a constant intmp
    class IntmpConst : public HD
    {
      public:
      IntmpConst(const mpz_class& v)
      : m_v(v)
      {}

      TaggedValue
      operator()(const Tuple& k)
      {
        return TaggedValue(TypedValue(TransLucid::Intmp(m_v),
                           TYPE_INDEX_INTMP), k);
      }

      uuid
      addExpr(const Tuple& k, HD* h)
      {
        return nil_uuid();
      }

      private:
      mpz_class m_v;
    };

    class TypeConst : public ExprHD
    {
      public:

      TypeConst(size_t index)
      : ExprHD(0), m_index(index)
      {}

      TaggedValue
      operator()(const Tuple& k)
      {
        return TaggedValue(TypedValue(TypeType(m_index),
                           TYPE_INDEX_TYPE), k);
      }

      private:
      size_t m_index;
    };
  } //namespace ConstHD
} //namespace TransLucid

#endif // CONSTHD_HPP_INCLUDED
