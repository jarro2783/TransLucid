/* Types for type inference.
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

#include <tl/tyinf/type.hpp>
#include <tl/tyinf/type_error.hpp>
#include <tl/output.hpp>
#include <tl/system.hpp>

namespace TransLucid
{
namespace TypeInference
{

namespace
{

//builds lub with current and join, current could be TypeNothing
Type
build_lub_constructed(Type current, Type join)
{
  //this could all be replaced with a double dispatch
  if (variant_is_type<TypeNothing>(current))
  {
    return join;
  }

  TypeCBV* acbv = get<TypeCBV>(&current);
  TypeCBV* bcbv = get<TypeCBV>(&join);
  TypeIntension *ainten = get<TypeIntension>(&current);
  TypeIntension *binten = get<TypeIntension>(&join);

  Constant *aconst = nullptr;
  Constant *bconst = nullptr;

  TypeAtomic *aatom = nullptr;
  TypeAtomic *batom = nullptr;

  if (acbv != nullptr && bcbv != nullptr)
  {
    return TypeCBV
      {
        construct_glb(acbv->lhs, bcbv->lhs),
        construct_lub(acbv->rhs, bcbv->rhs)
      };
  }

  if (ainten != nullptr && binten != nullptr)
  {
    return TypeIntension
      {
        construct_lub(ainten->body, binten->body)
      };
  }

  aconst = get<Constant>(&current);
  bconst = get<Constant>(&join);

  if (aconst != nullptr && bconst != nullptr)
  {
    if ((aconst->index() == bconst->index()))
    {
      if (*aconst == *bconst)
      {
        return *aconst;
      }
      else
      {
        return TypeAtomic
          {
            U"",
            aconst->index()
          };
      }
    }
    else
    {
      return TypeTop();
    }
  }

  aatom = get<TypeAtomic>(&current);
  batom = get<TypeAtomic>(&join);

  if (aconst != nullptr && batom != nullptr && aconst->index() == batom->index)
  {
    return *batom;
  }

  if (aatom != nullptr && bconst != nullptr && aatom->index == bconst->index())
  {
    return *aatom;
  }

  if (aatom != nullptr && batom != nullptr)
  {
    if (aatom->index == batom->index)
    {
      return *aatom;
    }
    else
    {
      return TypeTop();
    }
  }

  throw BoundInvalid{BoundInvalid::LUB, current, join};
}

template <typename T>
struct is_union_valid;

template <>
struct is_union_valid<Constant>
{
  typedef void type;
};

template <>
struct is_union_valid<TypeAtomic>
{
  typedef void type;
};

struct GLBConstructed
{
  typedef Type result_type;

  template <typename A, typename B>
  Type
  invalid(const A& a, const B& b) const
  {
    throw BoundInvalid{BoundInvalid::GLB, a, b};
  }

  template <typename A, typename B>
  Type
  operator()(const A& a, const B& b) const
  {
    return invalid(a, b);
  }

  template <typename T>
  Type
  operator()(const TypeNothing& n, const T& t) const
  {
    return t;
  }

  Type
  operator()(const TypeCBV& a, const TypeCBV& b) const
  {
    return TypeCBV
      {
        construct_lub(a.lhs, b.lhs),
        construct_glb(a.rhs, b.rhs)
      };
  }

  Type
  operator()(const TypeIntension& a, const TypeIntension& b) const
  {
    return TypeIntension
      {
        construct_glb(a.body, b.body)
      };
  }

  Type
  operator()(const TypeBase& a, const TypeBase& b) const
  {
    if (a.lhs.size() == b.lhs.size())
    {
      std::vector<Type> lhs;
      for (size_t i = 0; i != a.lhs.size(); ++i)
      {
        lhs.push_back(construct_lub(a.lhs.at(i), b.lhs.at(i)));

      }
      
      return TypeBase
        {
          lhs,
          construct_glb(a.rhs, b.rhs)
        };

    }
    else
    {
      return invalid(a, b);
    }
  }

  Type
  operator()(const Constant& a, const Constant& b) const
  {
    if (a == b)
    {
      return a;
    }
    else
    {
      return TypeBot();
    }
  }

  Type
  operator()(const TypeAtomic& a, const TypeAtomic& b) const
  {
    if (a.index == b.index)
    {
      return a;
    }
    else
    {
      return TypeBot();
    }
  }

  Type
  operator()(const TypeAtomic& a, const Constant& b) const
  {
    if (b.index() == a.index)
    {
      return b;
    }
    else
    {
      return TypeBot();
    }
  }

  Type
  operator()(const Constant& a, const TypeAtomic& b) const
  {
    return operator()(b, a);
  }

  Type
  operator()(const TypeAtomicUnion& a, const TypeAtomicUnion& b) const
  {
    return TypeAtomicUnion::intersection(a, b);
  }

  Type
  join_union(const TypeAtomicUnion& a, const Constant& b)
  {
    if (a.in(b))
    {
      return b;
    }
    else
    {
      return TypeBot();
    }
  }

  Type
  join_union(const TypeAtomicUnion& a, const TypeAtomic& b)
  {
    if (a.in(b))
    {
      return b;
    }
    else
    {
      return TypeBot();
    }
  }

  template <typename T, typename Dummy = typename is_union_valid<T>::type>
  Type
  operator()(const TypeAtomicUnion& a, const T& b)
  {
    return join_union(a, b);
  }

  template <typename T, typename Dummy = typename is_union_valid<T>::type>
  Type
  operator()(const T& a, const TypeAtomicUnion& b)
  {
    return join_union(b, a);
  }
};

Type
build_glb_constructed(const Type& current, const Type& join)
{
  GLBConstructed construct;
  return apply_visitor_double(construct, current, join);

  #if 0
  if (variant_is_type<TypeNothing>(current))
  {
    return join;
  }

  TypeCBV* acbv = get<TypeCBV>(&current);
  TypeCBV* bcbv = get<TypeCBV>(&join);
  TypeIntension *ainten = get<TypeIntension>(&current);
  TypeIntension *binten = get<TypeIntension>(&join);
  TypeBase* abase = get<TypeBase>(&current);
  TypeBase* bbase = get<TypeBase>(&join);

  Constant *aconst = nullptr;
  Constant *bconst = nullptr;

  TypeAtomic *aatom = nullptr;
  TypeAtomic *batom = nullptr;

  if (acbv != nullptr && bcbv != nullptr)
  {
    return TypeCBV
      {
        construct_lub(acbv->lhs, bcbv->lhs),
        construct_glb(acbv->rhs, bcbv->rhs)
      };
  }

  if (ainten != nullptr && binten != nullptr)
  {
    return TypeIntension
      {
        construct_glb(ainten->body, binten->body)
      };
  }

  if (abase != nullptr && bbase != nullptr && 
      abase->lhs.size() == bbase->lhs.size())
  {
    std::vector<Type> lhs;
    for (size_t i = 0; i != abase->lhs.size(); ++i)
    {
      lhs.push_back(construct_lub(abase->lhs.at(i), bbase->lhs.at(i)));

      return TypeBase
      {
        lhs,
        construct_glb(abase->rhs, bbase->rhs)
      };
    }
  }

  aconst = get<Constant>(&current);
  bconst = get<Constant>(&join);

  if (aconst != nullptr && bconst != nullptr)
  {
    if ((aconst->index() == bconst->index()))
    {
      return TypeAtomic
        {
          U"",
          aconst->index()
        };
    }
    else
    {
      return TypeBot();
    }
  }

  aatom = get<TypeAtomic>(&current);
  batom = get<TypeAtomic>(&join);

  if (aconst != nullptr && batom != nullptr && aconst->index() == batom->index)
  {
    return *aconst;
  }

  if (aatom != nullptr && bconst != nullptr && aatom->index == bconst->index())
  {
    return *bconst;
  }

  if (aatom != nullptr && batom != nullptr)
  {
    if (aatom->index == batom->index)
    {
      return *aatom;
    }
    else
    {
      return TypeBot();
    }
  }

  //we now have the glb of atomic unions, this is the intersection
  //of the types in them
  //
  // c ⊓ U = c when c \in U
  // A ⊓ U = A when c \in U

  TypeAtomicUnion* aU = get<TypeAtomicUnion>(&current);
  TypeAtomicUnion* bU = get<TypeAtomicUnion>(&join);

  if (aU != nullptr && bU != nullptr)
  {
    return TypeAtomicUnion::intersection(*aU, *bU);
  }

  if (aU != nullptr || bU != nullptr)
  {
    TypeAtomicUnion* unionJoin = nullptr;
    Type* other = nullptr;

    if (aU != nullptr)
    {
      unionJoin = aU;
      other = &join;
    }
    else if (bU != nullptr)
    {
      unionJoin = bU;
      other = &current;
    }

    if ((aconst = get<Constant>(other)) != nullptr)
    {
      if (unionJoin->in(*aconst))
      {
        return *aconst;
      }
      else
      {
        return TypeBot();
      }
    }

    if ((aatom = get<TypeAtomic>(other)) != nullptr)
    {
      if (unionJoin->in(*aatom))
      {
        return *aatom;
      }
      else
      {
        return TypeBot();
      }
    }
  }

  throw BoundInvalid{BoundInvalid::GLB, current, join};
  #endif
}

}

Type
construct_lub(Type a, Type b)
{
  //look for top
  if (variant_is_type<TypeTop>(a) || variant_is_type<TypeTop>(b))
  {
    return TypeTop();
  }

  //look for bottom
  int valid = 0;
  Type join[2];

  if (!variant_is_type<TypeBot>(a))
  {
    join[valid] = a;
    ++valid;
  }

  if (!variant_is_type<TypeBot>(b))
  {
    join[valid] = b;
    ++valid;
  }

  //lub of nothing is bottom
  if (valid == 0)
  {
    return TypeBot();
  }

  //lub of one thing is that thing
  if (valid == 1)
  {
    return join[0];
  }

  //now we have some actual terms to work with
  //we will definately have a lub term now
  TypeLUB lub;

  for (size_t i = 0; i != 2; ++i)
  {
    TypeLUB* l = get<TypeLUB>(&join[i]);
    TypeVariable* v = get<TypeVariable>(&join[i]);

    if (l != nullptr)
    {
      lub.vars.insert(l->vars.begin(), l->vars.end());
    }
    else if (v != nullptr)
    {
      lub.vars.insert(*v);
    }
    else
    {
      //the type by this point cannot be top, bot, lub, glb, or TypeVariable,
      //so whatever it is belongs in the constructed spot
      lub.constructed = build_lub_constructed(lub.constructed, join[i]);
    }
  }

  //however we might be left with something that is just the constructed term
  if (lub.vars.size() == 0)
  {
    return lub.constructed;
  }
  else if (lub.vars.size() == 1 
           && variant_is_type<TypeNothing>(lub.constructed))
  {
    //and if there is only one variable, return that
    return *lub.vars.begin();
  }

  return lub;
}

Type
construct_glb(Type a, Type b)
{
  //look for bottom
  if (variant_is_type<TypeBot>(a) || variant_is_type<TypeBot>(b))
  {
    return TypeBot();
  }

  //look for top
  int valid = 0;
  Type join[2];

  if (!variant_is_type<TypeTop>(a))
  {
    join[valid] = a;
    ++valid;
  }

  if (!variant_is_type<TypeTop>(b))
  {
    join[valid] = b;
    ++valid;
  }

  //glb of nothing is top
  if (valid == 0)
  {
    return TypeTop();
  }

  //glb of one thing is that thing
  if (valid == 1)
  {
    return join[0];
  }

  //now we have some actual terms to work with
  //we will definately have a lub term now
  TypeGLB glb;

  for (size_t i = 0; i != 2; ++i)
  {
    TypeGLB* g = get<TypeGLB>(&join[i]);
    TypeVariable* v = get<TypeVariable>(&join[i]);

    if (g != nullptr)
    {
      glb.vars.insert(g->vars.begin(), g->vars.end());
    }
    else if (v != nullptr)
    {
      glb.vars.insert(*v);
    }
    else
    {
      //the type by this point cannot be top, bot, lub, glb, or TypeVariable,
      //so whatever it is belongs in the constructed spot
      glb.constructed = build_glb_constructed(glb.constructed, join[i]);
    }
  }

  //however we might be left with something that is just the constructed term
  if (glb.vars.size() == 0)
  {
    return glb.constructed;
  }
  else if (glb.vars.size() == 1 
           && variant_is_type<TypeNothing>(glb.constructed))
  {
    //and if there is only one variable, return that
    return *glb.vars.begin();
  }

  return glb;
}

// does t contain tp for negative types
bool
type_term_contains_neg(Type t, Type tp)
{
  auto glb = construct_glb(t, tp);

  return glb == t;
}

// does t contain tp for positive types
bool
type_term_contains_pos(Type t, Type tp)
{
  auto lub = construct_lub(t, tp);

  return lub == t;
}

u32string
TypePrinter::print(const Type& t)
{
  apply_visitor(*this, t);
  return m_result;
}

void
TypePrinter::operator()(const TypeNothing& n)
{
  m_result += U"_";
}

void
TypePrinter::operator()(const TypeTop& top)
{
  m_result += U"⊤";
}

void
TypePrinter::operator()(const TypeBot& bot)
{
  m_result += U"⊥";
}

void
TypePrinter::operator()(const TypeVariable& var)
{
  if (m_alpha)
  {
    m_result += get_alpha(var);
  }
  else
  {
    m_result += print_type_variable(var);
  }
}

void
TypePrinter::operator()(const Constant& c)
{
  m_result += m_system.printConstant(c);
}

void
TypePrinter::operator()(const TypeAtomic& atomic)
{
  std::ostringstream os;
  os << U"atomic<" + atomic.name + U",";
  os << atomic.index << ">";

  m_result += utf8_to_utf32(os.str());
}

void
TypePrinter::operator()(const TypeRegion& atomic)
{
  m_result += U"region";
}

void
TypePrinter::operator()(const TypeTuple& atomic)
{
  m_result += U"tuple";
}

void
TypePrinter::operator()(const TypeAtomicUnion& u)
{
  m_result += U"Union::";

  if (u.constants.size() != 0)
  {
    m_result += U"Constants = {";
  }
  for (const auto& c : u.constants)
  {
    operator()(c);
    m_result += U" ";
  }
  if (u.constants.size() != 0)
  {
    m_result += U"}";
  }

  if (u.atomics.size() != 0)
  {
    m_result += U"Base Types = {";
  }
  for (const auto& a : u.atomics)
  {
    std::ostringstream os;
    os << a << " ";
    auto s = os.str();
    m_result += u32string(s.begin(), s.end());
  }
  if (u.atomics.size() != 0)
  {
    m_result += U"}";
  }
}

void
TypePrinter::operator()(const TypeGLB& glb)
{
  m_result += U"⊓{";

  if (!variant_is_type<TypeNothing>(glb.constructed))
  {
    apply_visitor(*this, glb.constructed);

    if (glb.vars.size() != 0)
    {
      m_result += U", ";
    }
  }

  auto iter = glb.vars.begin();
  while (iter != glb.vars.end())
  {
    operator()(*iter);
    if (++iter != glb.vars.end())
    {
      m_result += U", ";
    }
  }

  //for (const auto& v : glb.vars)
  //{
  //  m_result += U", ";
  //  operator()(v);
  //}

  m_result += U"}";
}

void
TypePrinter::operator()(const TypeLUB& lub)
{
  m_result += U"⊔{";

  if (!variant_is_type<TypeNothing>(lub.constructed))
  {
    apply_visitor(*this, lub.constructed);

    if (lub.vars.size() != 0)
    {
      m_result += U", ";
    }
  }

  auto iter = lub.vars.begin();
  while (iter != lub.vars.end())
  {
    operator()(*iter);
    if (++iter != lub.vars.end())
    {
      m_result += U", ";
    }
  }

  m_result += U"}";
}

void
TypePrinter::operator()(const TypeIntension& inten)
{
  m_result += U"↑ ";
  apply_visitor(*this, inten.body);
}

void
TypePrinter::operator()(const TypeCBV& cbv)
{
  m_result += U"(";

  apply_visitor(*this, cbv.lhs);

  m_result += U" → ";

  apply_visitor(*this, cbv.rhs);

  m_result += U")";
}

void
TypePrinter::operator()(const TypeBase& base)
{
  m_result += U"((";

  auto iter = base.lhs.begin();

  if (iter != base.lhs.end())
  {
    apply_visitor(*this, *iter);
    ++iter;
  }

  while (iter != base.lhs.end())
  {
    m_result += U", ";
    apply_visitor(*this, *iter);
    ++iter;
  }

  m_result += U") ↦ ";

  apply_visitor(*this, base.rhs);
  m_result += U")";
}

u32string
TypePrinter::get_alpha(TypeVariable v)
{
  auto iter = m_alphamap.find(v);

  if (iter == m_alphamap.end())
  {
    auto next = m_alphamap.insert(std::make_pair(v, m_nextAlpha));

    increment_alpha();

    return next.first->second;
  }
  else
  {
    return iter->second;
  }
}

void
TypePrinter::increment_alpha()
{
  size_t digit = m_nextAlpha.size() - 1;

  while (true)
  {
    if (m_nextAlpha[digit] == U'z')
    {
      //if we have run out of digits we need to add one more
      if (digit == 0)
      {
        m_nextAlpha[digit] = U'a';
        m_nextAlpha = U'a' + m_nextAlpha;
        break;
      }
      else
      {
        //otherwise reset and go to the next digit
        m_nextAlpha[digit] = U'a';
        --digit;
      }
    }
    else
    {
      ++m_nextAlpha[digit];
      break;
    }
  }
}

u32string
print_type(const Type& t, System& system, bool alpha)
{
  TypePrinter p(system, alpha);
  return p.print(t);
}

u32string
print_type_variable(TypeVariable var)
{
  std::ostringstream os;
  os << "v_" << var;
  
  return utf8_to_utf32(os.str());
}

//std::tuple<u32string, u32string, u32string>
//display_scheme(const TypeScheme& t)
//{
//}

void
TypeAtomicUnion::add(const Type& t)
{
  if (variant_is_type<TypeAtomic>(t))
  {
    const auto& a = get<TypeAtomic>(t);

    atomics.insert(a.index);

    //keep normalised, i.e., remove any constants that are covered by an
    //atomic
    auto iter = atomic_map.find(a.index);
    if (iter != atomic_map.end())
    {
      for (const auto& v : iter->second)
      {
        constants.erase(v);
      }

      atomic_map.erase(iter);
    }
  }
  else if (variant_is_type<Constant>(t))
  {
    const Constant& c = get<Constant>(t);
    if (atomics.find(c.index()) == atomics.end())
    {
      auto result = constants.insert(c);

      if (result.second)
      {
        atomic_map[c.index()].push_back(result.first);
      }
    }
  }
}

bool
TypeAtomicUnion::in(const Type& t) const
{
  const Constant* c = nullptr;
  const TypeAtomic* a = nullptr;

  if ((c = get<Constant>(&t)) != nullptr &&
      (atomics.find(c->index()) != atomics.end() ||
       constants.find(*c) != constants.end()) 
    )
  {
    return true;
  }
  else if ((a = get<TypeAtomic>(&t)) != nullptr && 
            atomics.find(a->index) != atomics.end())
  {
    return true;
  }
  
  return false;
}

TypeAtomicUnion
TypeAtomicUnion::intersection
(
  const TypeAtomicUnion& a, 
  const TypeAtomicUnion& b
)
{
  TypeAtomicUnion result;

  std::set_intersection(a.constants.begin(), a.constants.end(),
    b.constants.begin(), b.constants.end(),
    std::inserter(result.constants, result.constants.end()));

  std::set_intersection(a.atomics.begin(), a.atomics.end(),
    b.atomics.begin(), b.atomics.end(),
    std::inserter(result.atomics, result.atomics.end()));

  return result;
}

}
}
