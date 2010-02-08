#ifndef FUNCTION_HPP_INCLUDED
#define FUNCTION_HPP_INCLUDED

#include <tl/hyperdaton.hpp>

namespace TransLucid
{
   class FunctionHD : public HD
   {
      public:

      TaggedValue
      operator()(const Tuple& k);

      void
      addExpr(const Tuple& k, AST::Expr* e);
   };
}

#endif // FUNCTION_HPP_INCLUDED
