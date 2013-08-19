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

#include <iostream>

namespace TransLucid
{

namespace TypeInference
{

namespace
{

// put the double visit code here until it works, then move it into variant

template <typename Visitor, typename Visitable>
struct DoubleVisitor
{
  typedef typename Visitor::result_type result_type;

  DoubleVisitor(Visitor&& visitor, Visitable&& visitable)
  : v(visitor)
  , visitable(visitable)
  {
  }

  template <typename T>
  result_type
  operator()(const T& t)
  {
    return apply_visitor(v, visitable, t);
  }

  private:

  Visitor& v;
  Visitable& visitable;
};

template 
<
  typename Visitor,
  typename Visitable1,
  typename Visitable2
>
typename Visitor::result_type
apply_visitor_double(Visitor&& visitor, Visitable1&& v1, Visitable2&& v2)
{
  DoubleVisitor<Visitor, Visitable1> v{
    std::forward<Visitor>(visitor), 
    std::forward<Visitable1>(v1)
  };

  return apply_visitor(v, std::forward<Visitable2>(v2));
}

//head comparison for conditional constraints
//this could perhaps go in type.hpp
struct HeadCompare
{
  typedef bool result_type;

  template <typename A, typename B>
  bool
  operator()(const A& a, const B& b)
  {
    return false;
  }

  bool
  operator()(const TypeCBV&, const TypeCBV&)
  {
    return true;
  }

  bool
  operator()(const Constant& a, const Constant& b)
  {
    return a == b;
  }
};

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
intrusive_ptr_add_ref(ConstraintGraph::ConditionalNode* p)
{
  ++p->counter;
}

void
intrusive_ptr_release(ConstraintGraph::ConditionalNode* p)
{
  if (--p->counter == 0)
  {
    p->m_owner->m_conditionals.erase(p);
    delete p;
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

  auto a_less = a_data->second.less;
  auto b_greater = b_data->second.greater; 

  add_less_closed(a, b);

  //add the constraints to the upper and lower bounds of a and b
  a_data->second.upper = construct_glb(a_data->second.upper, 
    b_data->second.upper);
  new_lower_closed(b_data, construct_lub(b_data->second.lower,
    a_data->second.lower));

  //for each bp in the greater of b, its lower is (lower bp) lub (lower a)
  for (const auto bp : b_data->second.greater)
  {
    auto bp_data = m_graph.find(bp);

    new_lower_closed(bp_data, construct_lub(bp_data->second.lower, 
      a_data->second.lower));
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

    new_lower_closed(bp_data, construct_lub(bp_data->second.lower, t));
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
      throw InvalidConstraint(constraint);
    }
  }
}

void
ConstraintGraph::add_conditional(const CondConstraint& cc)
{
  auto data = get_make_entry(cc.a);

  CondNodeP p(new ConditionalNode(cc.s, cc.lhs, cc.rhs, this));

  data->second.conditions.insert(p.get());

  //if the condition is already satisfied, add its implications
  check_single_conditional(data, p);

  //propagate to less thans
  for (const auto less : data->second.less)
  {
    auto ldata = get_make_entry(less);
    ldata->second.conditions.insert(p);
  }
}

void
ConstraintGraph::check_conditionals(decltype(m_graph)::iterator var)
{
  for (const auto& cc : var->second.conditions)
  {
    check_single_conditional(var, cc);
  }
}

void
ConstraintGraph::check_single_conditional
(
  decltype(m_graph)::iterator var,
  const CondNodeP& cc
)
{
  if (apply_visitor_double(HeadCompare(), cc->s, var->second.lower))
  {
    add_to_closure(Constraint{cc->lhs, cc->rhs});
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

  const auto& lessthan = iter->second.greater;
  
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

  //add any constraints in b to a
  for (const auto& p : b_data->second.conditions)
  {
    a_data->second.conditions.insert(p);
  }

  a_data->second.conditions.insert
  (
    b_data->second.conditions.begin(),
    b_data->second.conditions.end()
  );
}

void
ConstraintGraph::add_less_closed(TypeVariable a, TypeVariable b)
{
  auto a_data = get_make_entry(a);
  auto b_data = get_make_entry(b);

  //include a < a and b < b
  auto a_less = a_data->second.less;
  a_less.push_back(a);
  auto b_greater = b_data->second.greater; 
  b_greater.push_back(b);

  //we go through every pair of a less and b greater
  for (const auto ap : a_less)
  {
    for (const auto bp : b_greater)
    {
      add_less(ap, bp);
    }
  }
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
  const TypeAtomic* atomiclhs = nullptr;
  const TypeAtomic* atomicrhs = nullptr;
  const Constant* constant = nullptr;
  const TypeBase* baselhs = nullptr;
  const TypeBase* baserhs = nullptr;
  const TypeIntension* intenlhs = nullptr;
  const TypeIntension* intenrhs = nullptr;
  const TypeAtomicUnion* unionrhs = nullptr;

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
  else if ((constant = get<Constant>(&c.lhs)) != nullptr &&
           (atomicrhs = get<TypeAtomic>(&c.rhs)) != nullptr &&
            constant->index() == atomicrhs->index)
  {
    //nothing to do here
  }
  else if ((atomiclhs = get<TypeAtomic>(&c.lhs)) != nullptr &&
           (atomicrhs = get<TypeAtomic>(&c.rhs)) != nullptr &&
            atomiclhs->index == atomicrhs-> index)
  {
    //nothing to do here
  }
  else if ((unionrhs = get<TypeAtomicUnion>(&c.rhs)) != nullptr &&
            unionrhs->in(c.lhs))
  {
    //nothing to do here
  }
  else if (variant_is_type<Constant>(c.lhs) && 
           variant_is_type<TypeDim>(c.rhs) )
  {
    //nothing to do here
  }
  else if (variant_is_type<TypeRegion>(c.lhs) && 
           variant_is_type<TypeRegion>(c.rhs) )
  {
    //nothing to do here
  }
  else if (variant_is_type<TypeTuple>(c.lhs) && 
           variant_is_type<TypeTuple>(c.rhs) )
  {
    //nothing to do here
  }
  else if ((cbvlhs = get<TypeCBV>(&c.lhs)) != nullptr && 
           (cbvrhs = get<TypeCBV>(&c.rhs)) != nullptr)
  {
    subc(Constraint{cbvrhs->lhs, cbvlhs->lhs}, result);
    subc(Constraint{cbvlhs->rhs, cbvrhs->rhs}, result);
  }
  else if ((baselhs = get<TypeBase>(&c.lhs)) != nullptr &&
           (baserhs = get<TypeBase>(&c.rhs)) != nullptr &&
           baselhs->lhs.size() == baserhs->lhs.size())
  {
    auto li = baselhs->lhs.begin();
    auto ri = baserhs->lhs.begin();
    while (li != baselhs->lhs.end())
    {
      subc(Constraint{*ri, *li}, result);
      ++li;
      ++ri;
    }

    subc(Constraint{baselhs->rhs, baserhs->rhs}, result);
  }
  else if ((intenlhs = get<TypeIntension>(&c.lhs)) != nullptr &&
           (intenrhs = get<TypeIntension>(&c.rhs)) != nullptr)
  {
    subc(Constraint{intenlhs->body, intenrhs->body}, result);
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
    //lower bound
    result += print_type(var.second.lower, system) + U", ";
    //less than
    result += print_type_variable_list(var.second.less);
    result += U" ≤ ";
    //the var
    result += print_type_variable(var.first);
    result += U" ≤ ";
    //greater than
    result += print_type_variable_list(var.second.greater);
    //upper bound
    result += U", " + print_type(var.second.upper, system);

    //conditional constraints
    for (const auto& cc : var.second.conditions)
    {
      result += U"\n";
      result += print_type(cc->s, system) + U" ≤ " + 
        print_type(var.first, system);
      result += U" ? ";
      result += print_type(cc->lhs, system) + U" ≤ " 
        + print_type(cc->rhs, system);
    }

    result += U"\n";
  }

  return result;
}


Type
ConstraintGraph::upper(TypeVariable a) const
{
  auto iter = m_graph.find(a);

  if (iter == m_graph.end())
  {
    return TypeTop();
  }
  else
  {
    return iter->second.upper;
  }
}

Type
ConstraintGraph::lower(TypeVariable b) const
{
  auto iter = m_graph.find(b);

  if (iter == m_graph.end())
  {
    return TypeBot();
  }
  else
  {
    return iter->second.lower;
  }
}

void
ConstraintGraph::setUpper(TypeVariable a, Type t)
{
  auto iter = get_make_entry(a);

  iter->second.upper = t;
}

void
ConstraintGraph::setLower(TypeVariable a, Type t)
{
  auto iter = get_make_entry(a);

  iter->second.lower = t;
}


//when anything in S is less than any variable a in the graph, set
//gamma < a
void
ConstraintGraph::rewrite_less(TypeVariable gamma, const VarSet& S)
{
  for (const auto& v : m_graph)
  {
    bool found = false;
    auto iter = S.begin();
    while (!found && iter != S.end())
    {
      if (less(*iter, v.first))
      {
        found = true;
        add_less_closed(gamma, v.first);
      }
      ++iter;
    }
  }
}

//when any variable a is less than anything in S, set a < lambda
void
ConstraintGraph::rewrite_greater(TypeVariable lambda, const VarSet& S)
{
  for (const auto& v : m_graph)
  {
    bool found = false;
    auto iter = S.begin();
    while (!found && iter != S.end())
    {
      if (less(v.first, *iter))
      {
        found = true;
        add_less_closed(v.first, lambda);
      }
      ++iter;
    }
  }
}

void
ConstraintGraph::rewrite_lub_glb
(
  TypeVariable gamma, 
  const VarSet& S,
  TypeVariable lambda, 
  const VarSet& T
)
{
  for (auto s : S)
  {
    for (auto t : T)
    {
      if (s == t || less(s, t))
      {
        add_less_closed(gamma, lambda);
        break;
      }
    }
  }
}

void
ConstraintGraph::collect(const VarSet& neg, const VarSet& pos)
{
  VarSet toRemove;

  //only keep lower bounds if the variable is positive
  //only keep upper bounds if the variable is negative
  for (auto& v : m_graph)
  {
    //only keep < if it is neg < pos
    if (pos.find(v.first) != pos.end())
    {
      //throw out all greater if positive
      v.second.greater.clear();

      //remove the upper bound
      v.second.upper = TypeTop();

      //then only keep less if they are negative
      auto end = std::remove_if(v.second.less.begin(), v.second.less.end(),
        [&neg] (const TypeVariable& t)
        {
          return neg.find(t) == neg.end();
        }
      );

      v.second.less.erase(end, v.second.less.end());
    }
    else if (neg.find(v.first) != neg.end())
    {
      //throw out all less if negative
      v.second.less.clear();

      //remove the lower bound
      v.second.lower = TypeBot();

      //then only keep greater if they are positive
      auto end = std::remove_if(
      v.second.greater.begin(), v.second.greater.end(),
        [&pos] (const TypeVariable& t)
        {
          return pos.find(t) == pos.end();
        }
      );

      v.second.greater.erase(end, v.second.greater.end());
    }
    else
    {
      //we don't even need this one at all if neutral
      toRemove.insert(v.first);
    }
  }

  for (auto v : toRemove)
  {
    m_graph.erase(v);
  }

}

}

}
