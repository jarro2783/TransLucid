#ifndef EXPR_COMPILER_HPP_INCLUDED
#define EXPR_COMPILER_HPP_INCLUDED

#include <tl/expr.hpp>
#include <tl/ast.hpp>

namespace TransLucid
{

  class ExprCompiler : public AST::Visitor
  {
    public:

    ExprCompiler(HD* i);
    ~ExprCompiler();

    HD* compile(AST::Expr*);

    AST::Data* visitAtExpr(AST::AtExpr*, AST::Data*);
    AST::Data* visitBinaryOpExpr(AST::BinaryOpExpr*, AST::Data*);
    AST::Data* visitBooleanExpr(AST::BooleanExpr*, AST::Data*);
    AST::Data* visitBuildTupleExpr(AST::BuildTupleExpr*, AST::Data*);
    AST::Data* visitConstantExpr(AST::ConstantExpr*, AST::Data*);
    AST::Data* visitConvertExpr(AST::SpecialOpsExpr*, AST::Data*);
    AST::Data* visitDimensionExpr(AST::DimensionExpr*, AST::Data*);
    AST::Data* visitHashExpr(AST::HashExpr*, AST::Data*);
    AST::Data* visitIdentExpr(AST::IdentExpr*, AST::Data*);
    AST::Data* visitIfExpr(AST::IfExpr*, AST::Data*);
    AST::Data* visitIntegerExpr(AST::IntegerExpr*, AST::Data*);
    AST::Data* visitIsSpecialExpr(AST::SpecialOpsExpr*, AST::Data*);
    AST::Data* visitIsTypeExpr(AST::SpecialOpsExpr*, AST::Data*);
    AST::Data* visitPairExpr(AST::PairExpr*, AST::Data*);
    AST::Data* visitOpExpr(AST::OpExpr*, AST::Data*);
    AST::Data* visitRangeExpr(AST::RangeExpr*, AST::Data*);
    AST::Data* visitSpecialExpr(AST::SpecialExpr*, AST::Data*);
    AST::Data* visitStringExpr(AST::StringExpr*, AST::Data*);
    AST::Data* visitUcharExpr(AST::UcharExpr*, AST::Data*);
    AST::Data* visitUnaryExpr(AST::UnaryExpr*, AST::Data*);

    //the new boost variant visitors
    typedef HD* result_type;

    HD* operator()(const Tree::AtExpr& e);
    HD* operator()(bool b);
    HD* operator()(const mpz_class& i);
    HD* operator()(const u32string& s);

    private:
    HD* m_i;
  };

} //namespace TransLucid

#endif // EXPR_COMPILER_HPP_INCLUDED
