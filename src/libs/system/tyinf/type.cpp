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
      return TypeAtomic
        {
          U"",
          aconst->index()
        };
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

Type
build_glb_constructed(Type current, Type join)
{
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

namespace
{
  class TypePrinter
  {
    public:

    typedef void result_type;

    TypePrinter(System& s)
    : m_system(s)
    {
    }

    u32string
    print(const Type& t)
    {
      apply_visitor(*this, t);
      return m_result;
    }

    void
    operator()(const TypeNothing& n)
    {
      m_result += U"_";
    }

    void
    operator()(const TypeTop& top)
    {
      m_result += U"⊤";
    }

    void
    operator()(const TypeBot& bot)
    {
      m_result += U"⊥";
    }

    void
    operator()(const TypeVariable& var)
    {
      m_result += print_type_variable(var);
    }

    void
    operator()(const Constant& c)
    {
      m_result += m_system.printConstant(c);
    }

    void
    operator()(const TypeAtomic& atomic)
    {
      std::ostringstream os;
      os << U"atomic<" + atomic.name + U",";
      os << atomic.index << ">";

      m_result += utf8_to_utf32(os.str());
    }

    void
    operator()(const TypeRegion& atomic)
    {
      m_result += U"region";
    }

    void
    operator()(const TypeTuple& atomic)
    {
      m_result += U"tuple";
    }

    void
    operator()(const TypeAtomicUnion& u)
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
    operator()(const TypeGLB& glb)
    {
      m_result += U"⊓{";

      if (!variant_is_type<TypeNothing>(glb.constructed))
      {
        apply_visitor(*this, glb.constructed);
      }

      for (const auto& v : glb.vars)
      {
        m_result += U", ";
        operator()(v);
      }

      m_result += U"}";
    }

    void
    operator()(const TypeLUB& lub)
    {
      m_result += U"⊔{";

      if (!variant_is_type<TypeNothing>(lub.constructed))
      {
        apply_visitor(*this, lub.constructed);
      }

      for (const auto& v : lub.vars)
      {
        m_result += U", ";
        operator()(v);
      }

      m_result += U"}";
    }

    void
    operator()(const TypeIntension& inten)
    {
      m_result += U"↑ ";
      apply_visitor(*this, inten.body);
    }

    void
    operator()(const TypeCBV& cbv)
    {
      m_result += U"(";

      apply_visitor(*this, cbv.lhs);

      m_result += U" → ";

      apply_visitor(*this, cbv.rhs);

      m_result += U")";
    }

    void
    operator()(const TypeBase& base)
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

    void
    operator()(const TypeDim& dim)
    {
      m_result += U"dim(";

      apply_visitor(*this, dim.body);

      m_result += U")";
    }

    private:
    u32string m_result;
    System& m_system;
  };
}

u32string
print_type(const Type& t, System& system)
{
  TypePrinter p(system);
  return p.print(t);
}

u32string
print_type_variable(TypeVariable var)
{
  std::ostringstream os;
  os << "v_" << var;
  
  return utf8_to_utf32(os.str());
}

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
