#ifndef CONSTHD_HPP_INCLUDED
#define CONSTHD_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>

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
   } //namespace ConstHD
} //namespace TransLucid

#endif // CONSTHD_HPP_INCLUDED
