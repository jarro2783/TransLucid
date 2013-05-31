/* Dependency checking
   Copyright (C) 2013 Jarryd Beck

This file is part of TransLucid.

TransLucid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

TransLucid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TransLucid; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

#include <tl/dependencies.hpp>

namespace TransLucid
{

DependencyFinder::result_type
DependencyFinder::operator()(const bool& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Special& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const mpz_class& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const char32_t& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const u32string& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LiteralExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::DimensionExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::IdentExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HashSymbol& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HostOpExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::ParenExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::UnaryOpExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BinaryOpExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::MakeIntenExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::EvalIntenExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::IfExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::HashExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::RegionExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::TupleExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::AtExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LambdaExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::PhiExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BaseAbstractionExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::BangAppExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::LambdaAppExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::PhiAppExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::WhereExpr& e)

{
  return result_type();
}

DependencyFinder::result_type
DependencyFinder::operator()(const Tree::ConditionalBestfitExpr& e)

{
  return result_type();
}

}
