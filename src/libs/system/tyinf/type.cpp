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
  if (variant_is_type<TypeNothing>(current))
  {
    return join;
  }

  TypeCBV* acbv = get<TypeCBV>(&current);
  TypeCBV* bcbv = get<TypeCBV>(&join);
  TypeIntension *ainten = get<TypeIntension>(&current);
  TypeIntension *binten = get<TypeIntension>(&join);

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
      m_result += U"atomic<" + atomic.name + U">";
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

}
}
