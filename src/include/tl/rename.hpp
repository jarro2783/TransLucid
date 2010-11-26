#include <tl/ast.hpp>

namespace TransLucid
{
  class RenameIdentifier
  {
    public:

    template <typename T>
    Tree::Expr 
    operator()(const T& e)
    {
      return e;
    }
  };
}
