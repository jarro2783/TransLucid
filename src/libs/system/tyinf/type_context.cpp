/* Type context.
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

#include <tl/tyinf/constraint_graph.hpp>
#include <tl/tyinf/type_context.hpp>
#include <tl/system.hpp>

#include <sstream>

namespace TransLucid
{

namespace TypeInference
{

namespace
{

template <typename T>
void
joinDims(T& jointo, const T& other)
{
  for (const auto& c : other)
  {
    auto iter = jointo.find(c.first);

    if (iter != jointo.end())
    {
      iter->second.first = construct_lub(iter->second.first, c.second.first);
      iter->second.second = construct_glb(iter->second.second, 
        c.second.second);
    }
    else
    {
      jointo.insert(c);
    }
  }

}

}

void
TypeContext::add(dimension_index d, Type t)
{
  m_lambdas[d] = t;
}

void
TypeContext::add(const u32string& x, Type t)
{
  m_vars[x] = t;
}

void
TypeContext::join(const TypeContext& other)
{
  for (const auto& p : other.m_lambdas)
  {
    auto iter = m_lambdas.find(p.first);

    if (iter != m_lambdas.end())
    {
      iter->second = construct_glb(p.second, iter->second);
    }
    else
    {
      m_lambdas.insert(p);
    }
  }

  for (const auto& p : other.m_vars)
  {
    auto iter = m_vars.find(p.first);

    if (iter != m_vars.end())
    {
      iter->second = construct_glb(p.second, iter->second);
    }
    else
    {
      m_vars.insert(p);
    }
  }

  joinDims(m_constDims, other.m_constDims);
  //joinDims(m_paramDims, other.m_paramDims);

  for (const auto& p : other.m_paramDims)
  {
    addParamDim(p.first, std::get<0>(p.second), std::get<1>(p.second),
      std::get<2>(p.second)
    );
  }
}

Type
TypeContext::lookup(dimension_index d)
{
  auto iter = m_lambdas.find(d);
  if (iter != m_lambdas.end())
  {
    return iter->second;
  }
  else
  {
    return TypeTop();
  }
}

Type
TypeContext::lookup(const u32string& x)
{
  auto iter = m_vars.find(x);

  if (iter != m_vars.end())
  {
    return iter->second;
  }
  else
  {
    return TypeTop();
  }
}

std::pair<Type, Type>
TypeContext::lookup(const Constant& c)
{
  auto iter = m_constDims.find(c);

  if (iter == m_constDims.end())
  {
    return std::make_pair(TypeBot(), TypeTop());
  }
  else
  {
    return iter->second;
  }
}

bool
TypeContext::has_entry(dimension_index d)
{
  return m_lambdas.find(d) != m_lambdas.end();
}

bool
TypeContext::has_entry(const u32string& x)
{
  return m_vars.find(x) != m_vars.end();
}

template <typename Printer>
u32string
TypeContext::print_internal(Printer&& p, System& system) const
{
  u32string result;

  for (const auto& l : m_lambdas)
  {
    std::ostringstream os;
    os << l.first;
    result += U"(" + utf8_to_utf32(os.str()) + U", " + 
      p(l.second) + U")\n";
  }

  for (const auto& v : m_vars)
  {
    result += U"(" + v.first + U", " + p(v.second) + U")\n";
  }

  for (const auto& d : m_constDims)
  {
    result += 
      p(d.second.first) + U" ≤ " +
      U"#." + system.printConstant(d.first) + U" ≤ " +
      p(d.second.second) + U"\n";
  }

  for (const auto& d : m_paramDims)
  {
    std::ostringstream os;
    os << d.first;

    result += 
      p(std::get<1>(d.second)) + U" ≤ " +
      U"#.(φ{" + utf8_to_utf32(os.str()) + U"} = "
      + p(std::get<0>(d.second)) + U") ≤ " 
      + p(std::get<2>(d.second)) + U"\n";
  }

  return result;
}

u32string
TypeContext::print_context(System& system) const
{
  using std::placeholders::_1;
  return print_internal(std::bind(&print_type, _1, std::ref(system), false), 
    system);

#if 0
  u32string result;

  for (const auto& l : m_lambdas)
  {
    std::ostringstream os;
    os << l.first;
    result += U"(" + utf8_to_utf32(os.str()) + U", " + 
      print_type(l.second, system) + U")\n";
  }

  for (const auto& v : m_vars)
  {
    result += U"(" + v.first + U", " + print_type(v.second, system) + U")\n";
  }

  for (const auto& d : m_constDims)
  {
    result += U"(" + system.printConstant(d.first) + U", ("
      + print_type(d.second.first, system) + U", " 
      + print_type(d.second.second, system) + U"))\n";
  }

  for (const auto& d : m_paramDims)
  {
    std::ostringstream os;
    os << d.first;

    result += U"(" + utf8_to_utf32(os.str()) + U", ("
      + print_type(std::get<0>(d.second), system) + U", " 
      + print_type(std::get<1>(d.second), system) + U", " 
      + print_type(std::get<2>(d.second), system)
      + U"))\n";
  }

  return result;
#endif
}

u32string
TypeContext::print_display(TypePrinter& printer, System& s) const
{
  return print_internal([&] (const Type& t) -> u32string
    {
      return printer.print(t);
    }, 
    s);
}

void
TypeContext::fix_context(ConstraintGraph& C)
{
  //add subc(a, b) for each pair of dimensions with types (a, b)
  std::vector<Constraint> constraints;

  for (const auto& d : m_constDims)
  {
    subc(Constraint{d.second.first, d.second.second}, constraints);
  }

  for (const auto& d : m_paramDims)
  {
    subc(Constraint{std::get<1>(d.second), std::get<2>(d.second)}, 
      constraints);
  }

  for (const auto& c : constraints)
  {
    C.add_to_closure(c);
  }
}

#if 0
//for the parameter given, if it is in m_paramDims, move it to
//m_constDims with the given value
void
TypeContext::instantiateDim(dimension_index param, const Constant& value)
{
  auto iter = m_paramDims.find(param);

  if (iter != m_paramDims.end())
  {
    addConstantDim(value, iter->second.first, iter->second.second);
  }
}
#endif

void
TypeContext::instantiate_parameters(ConstraintGraph& C)
{
  auto iter = m_paramDims.begin();

  while (iter != m_paramDims.end())
  {
    const auto& d = *iter;

    bool increment = true;

    //the value of the dimension must have a unique lower bound
    const TypeVariable* v = get<TypeVariable>(&std::get<0>(d.second));

    if (v != nullptr)
    {
      Type lower = C.lower(*v);
      Constant* thedim = get<Constant>(&lower);

      //then we can instantiate it as that dimension, which will compute the
      //lub and glb of the appropriate types if that dimension already exists
      if (thedim != nullptr && C.predecessor(*v).size() == 0)
      {
        if (thedim->index() == TYPE_INDEX_DIMENSION)
        {
          std::cout << "dimension " << get_constant<dimension_index>(*thedim)
            << " being instantiated" << std::endl;
          addConstantDim(*thedim, std::get<1>(d.second), std::get<2>(d.second));
          //add(get_constant<dimension_index>(*thedim), std::get<2>(d.second));
          m_paramDims.erase(iter++);
        }
        else
        {
          addConstantDim(*thedim, std::get<1>(d.second), std::get<2>(d.second));
          m_paramDims.erase(iter++);
        }
        increment = false;
      }
    }

    if (increment)
    {
      ++iter;
    }
  }

  fix_context(C);
}

void
TypeContext::remove_tl_context(const ConstraintGraph& C)
{
  //only remove the context when the lower bound variable has a non bottom
  //lower bound
  auto iter = m_constDims.begin();
  while (iter != m_constDims.end())
  {
    const auto& d = *iter;

    //here we assume that it is a type variable because we only run this
    //function on simplified type schemes
    auto lower = C.lower(get<TypeVariable>(d.second.first));
    
    if (get<TypeBot>(&lower) == nullptr)
    {
      iter = m_constDims.erase(iter); 
    }
    else
    {
      ++iter;
    }
  }
}

}
}
