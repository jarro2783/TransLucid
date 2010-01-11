#ifndef HYPERDATON_HPP_INCLUDED
#define HYPERDATON_HPP_INCLUDED

#include <tl/types.hpp>
//#include <tl/interpreter.hpp>

namespace TransLucid {

   class Interpreter;

   class HD {
      public:
      virtual ~HD() {}
      virtual TaggedValue operator()(const Tuple& k) = 0;
   };

} //namespace TransLucid

#endif // HYPERDATON_HPP_INCLUDED
