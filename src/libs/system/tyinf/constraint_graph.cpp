/* Constraint graph for type inference.
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

#include <algorithm>

#include <tl/tyinf/constraint_graph.hpp>
#include <tl/tyinf/type_error.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace TypeInference
{

namespace
{

bool
is_constructed(const Type& t)
{
  return !(
    variant_is_type<TypeVariable>(t) ||
    variant_is_type<TypeBot>(t) ||
    variant_is_type<TypeTop>(t) ||
    variant_is_type<TypeLUB>(t) ||
    variant_is_type<TypeGLB>(t)
  );
    
}

bool
is_elementary(const Constraint& c)
{
  if (variant_is_type<TypeVariable>(c.lhs) &&
      variant_is_type<TypeVariable>(c.rhs))
  {
    return true;
  }

  if (variant_is_type<TypeVariable>(c.lhs))
  {
    return is_constructed(c.rhs);
  }

  if (variant_is_type<TypeVariable>(c.rhs))
  {
    return is_constructed(c.lhs);
  }

  return false;
}

}

void
ConstraintGraph::add_constraint(TypeVariable a, TypeVariable b, 
  ConstraintQueue& q)
{
  if (less(a, b))
  {
    return ;
  }

  // find a' and b' | a' < a and b < b'

  auto a_data = get_make_entry(a);
  auto b_data = get_make_entry(b);

  std::vector<Constraint> toadd;
  subc(Constraint{a_data->second.lower, b_data->second.upper}, toadd);

  push_range(toadd.begin(), toadd.end(), q);

  //we go through every pair of a less and b greater
  for (const auto ap : a_data->second.greater)
  {
    for (const auto bp : b_data->second.less)
    {
      add_less(ap, bp);
    }
  }

  add_less(a, b);

  //for each bp in the greater of b, its lower is (lower bp) lub (lower a)
  for (const auto bp : b_data->second.greater)
  {
    auto bp_data = m_graph.find(bp);

    bp_data->second.lower = construct_lub(bp_data->second.lower, 
      a_data->second.lower);
  }

  //for each ap in the less of a, its upper is (upper ap) glb (upper b)
  for (const auto ap : a_data->second.less)
  {
    auto ap_data = m_graph.find(ap);

    ap_data->second.upper = construct_glb(ap_data->second.upper,
      b_data->second.upper);
  }
}

void
ConstraintGraph::add_constraint(TypeVariable a, Type t, ConstraintQueue& q)
{
  auto iter = get_make_entry(a);

  if (type_term_contains_neg(iter->second.upper, t))
  {
    return ;
  }

  std::vector<Constraint> toadd;
  subc(Constraint{iter->second.lower, t}, toadd);

  push_range(toadd.begin(), toadd.end(), q);

  //put itself in the constraints
  iter->second.upper = construct_glb(iter->second.upper, t);

  //for each ap less than a, its upper is (upper ap) glb t
  for (const auto ap : iter->second.less)
  {
    auto ap_data = m_graph.find(ap);

    ap_data->second.upper = construct_glb(ap_data->second.upper, t);
  }
}

void
ConstraintGraph::add_constraint(Type t, TypeVariable b, ConstraintQueue& q)
{
  auto iter = get_make_entry(b);

  if (type_term_contains_pos(iter->second.lower, t))
  {
    return ;
  }

  std::vector<Constraint> toadd;
  subc(Constraint{t, iter->second.upper}, toadd);

  push_range(toadd.begin(), toadd.end(), q);

  //put itself in the constraints
  iter->second.lower = construct_lub(iter->second.lower, t);

  //for each bp greater than b, its lower is (lower bp) lub t
  for (const auto bp : iter->second.greater)
  {
    auto bp_data = m_graph.find(bp);

    bp_data->second.lower = construct_lub(bp_data->second.lower, t);
  }
}

//this one only works if t1 and t2 are one of the above three cases
//this is where the main algorithm happens, it takes care of calling subc
//and adding any extra constraints
void
ConstraintGraph::add_to_closure(const Constraint& constraint)
{
  ConstraintQueue q;
  q.push(constraint);

  while (!q.empty())
  {
    auto c = q.front();
    q.pop();

    auto tv1 = get<TypeVariable>(&c.lhs);
    auto tv2 = get<TypeVariable>(&c.rhs);

    if (tv1 != nullptr && tv2 != nullptr)
    {
      add_constraint(*tv1, *tv2, q);
    }
    else if (tv1 != nullptr)
    {
      add_constraint(*tv1, c.rhs, q);
    }
    else if (tv2 != nullptr)
    {
      add_constraint(c.lhs, *tv2, q);
    }
    else
    {
      throw "Attempt to add invalid constraint";
    }
  }
}

bool
ConstraintGraph::less(TypeVariable a, TypeVariable b) const
{
  auto iter = m_graph.find(a);

  if (iter == m_graph.end())
  {
    return false;
  }

  const auto& lessthan = iter->second.less;
  
  return std::binary_search(lessthan.begin(), lessthan.end(), b);
}

void
ConstraintGraph::add_less(TypeVariable a, TypeVariable b)
{
  //add b to the greater of a, and a to the less of b
  auto a_data = get_make_entry(a);
  auto b_data = get_make_entry(b);

  insert_sorted(a_data->second.greater, b);
  insert_sorted(b_data->second.less, a);
}

decltype(ConstraintGraph::m_graph)::iterator
ConstraintGraph::get_make_entry(TypeVariable a)
{
  auto iter = m_graph.find(a);

  if (iter == m_graph.end())
  {
    iter = m_graph.insert(std::make_pair(a, ConstraintNode())).first;
  }

  return iter;
}

void
subc(const Constraint& c, std::vector<Constraint>& result)
{
  const TypeLUB* lub = nullptr;
  const TypeGLB* glb = nullptr;
  const TypeCBV* cbvlhs = nullptr;
  const TypeCBV* cbvrhs = nullptr;

  if((lub = get<TypeLUB>(&c.lhs)) != nullptr)
  {
    for (const auto& t : lub->vars)
    {
      subc(Constraint{t, c.rhs}, result);
    }

    if (!variant_is_type<TypeNothing>(lub->constructed))
    {
      subc(Constraint{lub->constructed, c.rhs}, result);
    }
  }
  else if ((glb = get<TypeGLB>(&c.rhs)) != nullptr)
  {
    for (const auto& t : glb->vars)
    {
      subc(Constraint{c.lhs, t}, result);
    }

    if (!variant_is_type<TypeNothing>(glb->constructed))
    {
      subc(Constraint{c.lhs, glb->constructed}, result);
    }
  }
  else if (is_elementary(c))
  {
    result.push_back(c);
  }
  else if (variant_is_type<TypeTop>(c.rhs))
  {
    //there is actually nothing to do here
  }
  else if (variant_is_type<TypeBot>(c.lhs))
  {
    //there is actually nothing to do here
  }
  else if ((cbvlhs = get<TypeCBV>(&c.lhs)) != nullptr && 
           (cbvrhs = get<TypeCBV>(&c.rhs)) != nullptr)
  {
    subc(Constraint{cbvrhs->lhs, cbvlhs->lhs}, result);
    subc(Constraint{cbvlhs->rhs, cbvrhs->rhs}, result);
  }
  else
  {
    throw SubcInvalid{c};
  }
}

void
ConstraintGraph::make_union(const ConstraintGraph& other)
{
  m_graph.insert(other.m_graph.begin(), other.m_graph.end());
}

u32string
ConstraintGraph::print(System& system) const
{
  u32string result;
  for (const auto& var : m_graph)
  {
    //print less than
    result += print_type_variable_list(var.second.less);
    result += U", " + print_type(var.second.lower, system);
    result += U" ≤ ";
    result += print_type_variable(var.first);
    result += U" ≤ ";
    result += U", " + print_type(var.second.upper, system);
    result += print_type_variable_list(var.second.greater);

    result += U"\n";
  }

  return result;
}

}

}
