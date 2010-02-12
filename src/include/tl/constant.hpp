#ifndef CONSTANT_HPP_INCLUDED
#define CONSTANT_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <tl/interpreter.hpp>
#include <map>

namespace TransLucid
{

  //this hyperdaton varies in the type and value dimension
  //it can be added to by adding a parser for a specific type
  //it can be used by putting k = [type : t, value : v]
  class ConstantHD : public HD
  {
    public:

    ConstantHD(Interpreter& i)
    : m_i(i)
    {
    }

    TaggedValue
    operator()(const Tuple& k);

    void
    addExpr(const Tuple& k, AST::Expr* e);

    private:
    Interpreter& m_i;

    std::map<u32string, HD*> m_build;
  };

}

#endif // CONSTANT_HPP_INCLUDED
