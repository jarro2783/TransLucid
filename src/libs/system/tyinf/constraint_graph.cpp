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
#include <tl/utility.hpp>

namespace TransLucid
{

namespace TypeInference
{

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
ConstraintGraph::add_to_closure(Type t1, Type t2)
{
  ConstraintQueue q;
  q.push({t1, t2});

  while (!q.empty())
  {
    auto c = q.front();
    q.pop();

    auto tv1 = get<TypeVariable>(&t1);
    auto tv2 = get<TypeVariable>(&t2);

    if (tv1 != nullptr && tv2 != nullptr)
    {
      add_constraint(*tv1, *tv2, q);
    }
    else if (tv1 != nullptr)
    {
      add_constraint(*tv1, t2, q);
    }
    else if (tv2 != nullptr)
    {
      add_constraint(t1, *tv2, q);
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
}

}

}
