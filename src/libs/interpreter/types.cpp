/* TODO: Give a descriptor.
   Copyright (C) 2009, 2010 Jarryd Beck and John Plaice

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

#include <tl/types.hpp>
#include <boost/foreach.hpp>
//#include <boost/bind.hpp>
#include <tl/range.hpp>
#include <tl/interpreter.hpp>
#include <tl/exception.hpp>
#include <tl/header_type.hpp>
#include <tl/footer_type.hpp>

namespace TransLucid
{

#if 0

TypeRegistry::TypeRegistry(Interpreter& i)
: m_nextIndex(1), m_indexError(0), m_interpreter(i)
{
  #if 0
  TypeManager* m = new SpecialManager(*this);
  //makeOpTypeError =
  //  boost::bind(&TypeRegistry::makeOpTypeErrorActual, this, m);
  m_indexSpecial = m->index();
  m = new UnevalManager(*this);
  m_indexUneval = m->index();

  RangeManager* rm = new RangeManager(*this, "_range");
  m_indexRange = rm->index();
  rm->printName(false);

  m = new IntmpManager(*this, "intmp", false);
  m_indexIntmp = m->index();
  m = new BooleanManager(*this, "bool", false);
  m_indexBool = m->index();
  m = new TupleManager(*this, "tuple");
  m_indexTuple = m->index();
  m = new DimensionManager(*this, "_dimension");
  m_indexDimension = m->index();
  m = new ExprManager(*this, "expr");
  m_indexExpr = m->index();
  m = new StringManager(*this, "ustring");
  m_indexString = m->index();
  m = new CalcManager(*this, "_calc");
  m_indexCalc = m->index();
  m = new CharManager(*this, "uchar");
  m_indexChar = m->index();
  m = new EquationGuardManager(*this, "_eguard");
  m_indexGuard = m->index();
  m = new PairManager(*this, "_pair");
  m_indexPair = m->index();
  #endif
  //new HeaderManager<HeaderType::DIRECT>(*this);
}

TypeRegistry::~TypeRegistry()
{
}

#endif

//TypedValue TypeRegistry::makeOpTypeErrorActual(const TypeManager* special)
//{
//  return TypedValue(Special(Special::TYPEERROR), special->index());
//}

Tuple::Tuple()
: m_value(new tuple_t)
{
}

Tuple::Tuple(const tuple_t& tuple)
: m_value(new tuple_t(tuple))
{
}

Tuple
Tuple::insert(size_t key, const TypedValue& value) const
{
  tuple_t t = *m_value;
  t.insert(std::make_pair(key, value));
  return Tuple(t);
}

void
Tuple::print(std::ostream& os) const
{
  os << "tuple";
}

} //namespace TransLucid
