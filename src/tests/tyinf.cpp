/* Test code for type inference.
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

#include <tl/parser_api.hpp>
#include <tl/semantic_transform.hpp>
#include <tl/system.hpp>
#include <tl/tyinf/type_error.hpp>
#include <tl/tyinf/type_inference.hpp>
#include <tl/output.hpp>

void
inference(TransLucid::System& system);

void
infer(TransLucid::System& system, const TransLucid::u32string& expr);

void
cgraph(TransLucid::System& system);

int main(int argc, char *argv[])
{
  TransLucid::System system;

  try
  {
    cgraph(system);
    inference(system);
  }
  catch (const TransLucid::TypeInference::TypeError& e)
  {
    std::cerr << "Type error in type inference: " << e.print(system) 
      << std::endl;
  }
  catch (const char* c)
  {
    std::cerr << "Terminated with exception: " << c << std::endl;
  }

  return 0;
}

void
infer(TransLucid::System& system, const TransLucid::u32string& expr)
{
  std::cout << expr << std::endl;
  TransLucid::TypeInference::FreshTypeVars fresh;
  TransLucid::SemanticTransform transform(system);
  TransLucid::TypeInference::TypeInferrer infer(system, fresh);

  TransLucid::Tree::Expr e = TransLucid::Parser::parse_expr(system, 
    expr);

  auto et = transform.transform(e);

  auto t = infer.infer(et);

  std::cout << "after type inference\n" << 
    print_type(std::get<1>(t), system) << std::endl <<
    std::get<2>(t).print(system) << std::endl;
  
  auto canon = TransLucid::TypeInference::canonise(t, fresh);

  std::cout << "after canonisation\n" << 
    print_type(std::get<1>(canon), system) << std::endl <<
    std::get<2>(canon).print(system) << std::endl;

  auto collected = TransLucid::TypeInference::garbage_collect(canon);

  std::cout << "after garbage collection\n" << 
    print_type(std::get<1>(collected), system) << std::endl <<
    std::get<2>(collected).print(system) << std::endl;
}

void
inference(TransLucid::System& system)
{
  infer(system, U"42");
  infer(system, UR"*(\x -> x)*");
  infer(system, UR"*((\x -> x)!42)*");
  infer(system, UR"*(\x -> if true then x else x fi)*");
  infer(system, UR"*(if true then \x -> x else \x -> x fi)*");
  infer(system, UR"*((\_(x, y) -> y).(1, 2))*");
}

void
cgraph(TransLucid::System& system)
{
  using TransLucid::TypeInference::ConstraintGraph;
  using TransLucid::TypeInference::Constraint;
  using TransLucid::TypeInference::TypeCBV;

  std::cout << "closure test" << std::endl;

  ConstraintGraph C;

  C.add_to_closure(Constraint{1, 2});
  C.add_to_closure(Constraint{2, 3});
  C.add_to_closure(Constraint{TypeCBV{4, 5}, 1});
  C.add_to_closure(Constraint{3, TypeCBV{6, 7}});

  std::cout << C.print(system) << std::endl;
}
