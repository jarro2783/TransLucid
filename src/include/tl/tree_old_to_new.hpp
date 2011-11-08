#include <tl/ast.hpp>
#include <tl/ast-new.hpp>

#include <algorithm>

namespace TransLucid
{
  class TreeOldToNew
  {
    public:
    typedef Tree::Expr result_type;

    Tree::Expr operator()(const Tree::nil& n)
    {
      return Tree::nil();
    }

    Tree::Expr operator()(bool b)
    {
      return b;
    }

    Tree::Expr operator()(Special s)
    {
      return s;
    }

    Tree::Expr operator()(const mpz_class& i)
    {
      return i;
    }

    Tree::Expr operator()(char32_t c)
    {
      return c;
    }

    Tree::Expr operator()(const u32string& s)
    {
      return s;
    }

    Tree::Expr operator()(const Tree::HashSymbol& e)
    {
      return Tree::HashSymbol();
    }

    Tree::Expr operator()(const Tree::LiteralExpr& e)
    {
      return Tree::LiteralExpr(e.type, e.text);
    }

    Tree::Expr operator()(const Tree::DimensionExpr& e)
    {
      Tree::DimensionExpr dim;
      dim.text = e.text;
      dim.dim = e.dim;
      return dim;
    }

    Tree::Expr operator()(const Tree::IdentExpr& e)
    {
      return Tree::IdentExpr(e.text);
    }

    Tree::Expr operator()(const Tree::ParenExpr& e)
    {
      return Tree::ParenExpr(boost::apply_visitor(*this, e.e));
    }

    Tree::Expr operator()(const Tree::UnaryOpExpr& e)
    {
      Tree::UnaryType type = Tree::UNARY_PREFIX;
      switch(e.op.type)
      {
        case Tree::UNARY_PREFIX:
        type = Tree::UNARY_PREFIX;
        break;

        case Tree::UNARY_POSTFIX:
        type = Tree::UNARY_POSTFIX;
        break;
      }

      return Tree::UnaryOpExpr
      (
        Tree::UnaryOperator(e.op.op, e.op.symbol, type),
        boost::apply_visitor(*this, e.e)
      );
    }

    Tree::Expr operator()(const Tree::BinaryOpExpr& e)
    {
      const Tree::BinaryOperator& oldop = e.op;
      Tree::InfixAssoc assoc = Tree::ASSOC_LEFT;
      switch(oldop.assoc)
      {
        case Tree::ASSOC_LEFT:
        assoc = Tree::ASSOC_LEFT;
        break;

        case Tree::ASSOC_RIGHT:
        assoc = Tree::ASSOC_RIGHT;
        break;

        case Tree::ASSOC_NON:
        assoc = Tree::ASSOC_NON;
        break;

        case Tree::ASSOC_VARIABLE:
        assoc = Tree::ASSOC_VARIABLE;
        break;

        case Tree::ASSOC_COMPARISON:
        assoc = Tree::ASSOC_COMPARISON;
        break;
      }

      Tree::BinaryOperator newop(
        assoc, oldop.op, oldop.symbol, oldop.precedence);

      return Tree::BinaryOpExpr
      (
        newop, 
        boost::apply_visitor(*this, e.lhs), 
        boost::apply_visitor(*this, e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::IfExpr& e)
    {
      std::vector<std::pair<Tree::Expr, Tree::Expr>> else_ifs;

      for (const auto p : e.else_ifs)
      {
        else_ifs.push_back(std::make_pair(
          boost::apply_visitor(*this, p.first),
          boost::apply_visitor(*this, p.second)
        ));
      }

      return Tree::IfExpr
      (
        boost::apply_visitor(*this, e.condition),
        boost::apply_visitor(*this, e.then),
        else_ifs,
        boost::apply_visitor(*this, e.else_)
      );
    }

    Tree::Expr operator()(const Tree::HashExpr& e)
    {
      return Tree::HashExpr(boost::apply_visitor(*this, e.e));
    }

    Tree::Expr operator()(const Tree::TupleExpr& e)
    {
      Tree::TupleExpr::TuplePairs pairs;

      std::transform(e.pairs.begin(), e.pairs.end(), std::back_inserter(pairs),
        [this] (const decltype(e.pairs)::value_type& p)
        {
          return std::make_pair
          (
            boost::apply_visitor(*this, p.first),
            boost::apply_visitor(*this, p.second)
          );
        }
      );

      return Tree::TupleExpr(pairs);
    }

    Tree::Expr operator()(const Tree::AtExpr& e)
    {
      return Tree::AtExpr
      (
        boost::apply_visitor(*this, e.lhs),
        boost::apply_visitor(*this, e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::BangExpr& e)
    {
      Tree::BangExpr newbang(e.name, boost::apply_visitor(*this, e.rhs));
      newbang.argDim = e.argDim;
      newbang.scope = e.scope;

      return newbang;
    }

    Tree::Expr operator()(const Tree::LambdaExpr& e)
    {
      Tree::LambdaExpr lamb(std::vector<Tree::Expr>(),
        e.name, boost::apply_visitor(*this, e.rhs));
      lamb.argDim = e.argDim;
      lamb.scope = e.scope;

      return lamb;
    }

    Tree::Expr operator()(const Tree::PhiExpr& e)
    {
      Tree::PhiExpr phi(std::vector<Tree::Expr>(),
        e.name, boost::apply_visitor(*this, e.rhs));
      phi.argDim = e.argDim;
      phi.odometerDim = e.odometerDim;
      phi.scope = e.scope;

      return phi;
    }

    Tree::Expr operator()(const Tree::BangAppExpr& e)
    {
      std::vector<Tree::Expr> args;
      std::transform(e.args.begin(), e.args.end(), std::back_inserter(args),
        [this] (const Tree::Expr& old)
        {
          return boost::apply_visitor(*this, old);
        }
      );

      return Tree::BangAppExpr(
        boost::apply_visitor(*this, e.name),
        args);
    }

    Tree::Expr operator()(const Tree::LambdaAppExpr& e)
    {
      return Tree::LambdaAppExpr
      (
        boost::apply_visitor(*this, e.lhs),
        boost::apply_visitor(*this, e.rhs)
      );
    }

    Tree::Expr operator()(const Tree::PhiAppExpr& e)
    {
      Tree::PhiAppExpr phiapp(
        boost::apply_visitor(*this, e.lhs),
        boost::apply_visitor(*this, e.rhs)
      );

      phiapp.Lall = e.Lall;

      return phiapp;
    }

    Tree::Expr operator()(const Tree::WhereExpr& e)
    {
      Tree::WhereExpr where;
      where.e = boost::apply_visitor(*this, e.e);
      where.name = e.name;

      std::transform(e.dims.begin(), e.dims.end(), 
        std::back_inserter(where.dims),
        [this] (const decltype(e.dims)::value_type& p)
        {
          return std::make_pair(p.first, 
            boost::apply_visitor(*this, p.second));
        }
      );
      
      std::transform(e.vars.begin(), e.vars.end(), 
        std::back_inserter(where.vars),
        [this] (const decltype(e.vars)::value_type& eqn)
        {
          return std::make_tuple
          (
            std::get<0>(eqn),
            boost::apply_visitor(*this, std::get<1>(eqn)),
            boost::apply_visitor(*this, std::get<2>(eqn)),
            boost::apply_visitor(*this, std::get<3>(eqn))
          );
        }
      );

      where.myDim = e.myDim;
      where.Lin = e.Lin;
      where.Lout = e.Lout;
      where.whichDims = e.whichDims;

      return where;
    }
  };
}
