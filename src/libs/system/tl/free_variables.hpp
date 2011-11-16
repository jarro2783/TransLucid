#include <vector>
#include <utility>

#include <tl/ast_fwd.hpp>
#include <tl/types_basic.hpp>

namespace TransLucid
{
  class FreeVariableReplacer
  {
    typedef std::vector<std::pair<u32string, dimension_index>> result_type;

    result_type operator()(const Tree::IdentExpr& expr);
  };
}
