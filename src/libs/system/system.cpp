/* System hyperdaton.
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

#include <tl/system.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>
#include <tl/valuehd.hpp>
//#include <tl/compiled_functors.hpp>
#include <algorithm>
#include <tl/consthd.hpp>
#include <tl/translator.hpp>

namespace TransLucid
{

namespace
{

  class UniqueWS : public WS
  {
    public:

    UniqueWS(int start)
    : m_index(start)
    {
    }

    TaggedConstant
    operator()(const Tuple& k)
    {
      return TaggedConstant(Constant(Intmp(m_index++), TYPE_INDEX_INTMP), k);
    }

    private:
    mpz_class m_index;
  };

  class DimensionsStringWS : public WS
  {
    public:

    DimensionsStringWS(DimensionTranslator& d)
    : m_d(d)
    {}

    TaggedConstant
    operator()(const Tuple& k)
    {
      Tuple::const_iterator iter = k.find(DIM_TEXT);
      if (iter == k.end())
      {
        throw "called dim lookup without text dimension";
      }
      return TaggedConstant(Constant(Intmp(
        m_d.lookup(iter->second.value<String>().value())),
                   TYPE_INDEX_INTMP), k);
    }

    private:
    DimensionTranslator& m_d;
  };

  class DimensionsTypedWS : public WS
  {
    public:

    DimensionsTypedWS(DimensionTranslator& d)
    : m_d(d)
    {}

    TaggedConstant
    operator()(const Tuple& k)
    {
      Tuple::const_iterator iter = k.find(DIM_VALUE);
      if (iter == k.end())
      {
        throw "called dim lookup without value dimension";
      }
      return TaggedConstant(Constant(Intmp(
        m_d.lookup(iter->second)), TYPE_INDEX_INTMP), k);
    }

    private:
    DimensionTranslator& m_d;
  };

  class UniqueDimensionWS : public WS
  {
    public:

    UniqueDimensionWS(DimensionTranslator& d)
    : m_d(d)
    {
    }

    TaggedConstant
    operator()(const Tuple& k)
    {
      size_t i = m_d.unique();
      return TaggedConstant(Constant(Intmp(i), TYPE_INDEX_INTMP), k);
    }

    private:
    DimensionTranslator& m_d;
  };
}

template <typename T>
WS*
System::buildConstantWS(size_t index)
{
  WS* h = new T(this);

  tuple_t guard =
  {
    {
      DIM_TYPE,
      Constant(String(T::name), TYPE_INDEX_USTRING)
    }
  };

  //sets the following

  //CONST | [type : ustring<name>]
  addEquation(U"CONST", GuardWS(Tuple(guard)), h);

  //TYPE_INDEX | [type : ustring<name>] = index;;
  addEquation
  (
    U"TYPE_INDEX", 
    GuardWS(Tuple(guard)), 
    new Hyperdatons::IntmpConstWS(index)
  );
  return h;
}

void
System::init_types()
{
  for(auto v : builtin_name_to_index)
  {
    addEquation(v.first, GuardWS(),
                        new Hyperdatons::TypeConstWS(v.second));
  }
}

System::System()
: m_time(0),
  builtin_name_to_index
  {
   {U"ustring", TYPE_INDEX_USTRING},
   {U"intmp", TYPE_INDEX_INTMP},
   {U"bool", TYPE_INDEX_BOOL},
   {U"special", TYPE_INDEX_SPECIAL},
   {U"uchar", TYPE_INDEX_UCHAR},
   {U"dim", TYPE_INDEX_DIMENSION},
   {U"type", TYPE_INDEX_TYPE}
  }
{
  //create the obj, const and fun ids

  //we need dimensions and unique to do anything
  addEquation(U"DIMENSION_NAMED_INDEX", GuardWS(),
                      new DimensionsStringWS(m_dimTranslator));
  addEquation(U"DIMENSION_VALUE_INDEX", GuardWS(),
                      new DimensionsTypedWS(m_dimTranslator));
  addEquation
  (
    U"_unique", GuardWS(),
    new UniqueWS(std::max
    (
      static_cast<int>(TYPE_INDEX_LAST),
      static_cast<int>(DIM_INDEX_LAST)
    ))
  );

  addEquation
  (
    U"_uniquedim", GuardWS(),
    new UniqueDimensionWS(m_dimTranslator)
  );

  //add this
  addEquation(U"this", GuardWS(), this);

  //add variables for all the types
  //std::vector<ustring_t> typeNames = {"intmp", "uchar"};
  init_types();

  //build the constant creators
  buildConstantWS<Hyperdatons::BoolWS>(TYPE_INDEX_BOOL);
  buildConstantWS<Hyperdatons::UCharWS>(TYPE_INDEX_UCHAR);
  WS* intmpWS = buildConstantWS<Hyperdatons::IntmpWS>(TYPE_INDEX_INTMP);
  buildConstantWS<Hyperdatons::UStringWS>(TYPE_INDEX_USTRING);

  //set this as the default int too
  addEquation(U"DEFAULTINT", GuardWS(), intmpWS);

  m_translator = new Translator(*this);
}

System::~System()
{
  delete m_translator;
}

void
System::tick()
{
  #if 0
  tuple_t current;
  current[DIM_TIME] = Constant(Intmp(m_time), TYPE_INDEX_INTMP);

  tuple_t toSave = current;

  Tuple k(current);

  //run the applicable demands

  for(const DemandStore::value_type& d : m_demands)
  {
    Tuple t = d.second.evaluate(Tuple());
    if (tupleApplicable(t, k))
    {
      IOList::const_iterator iter = m_variables.find(d.first);
      if (iter != m_variables.end())
      {
        //evaluate the demand
        TaggedConstant r = (*iter->second)(k);
        //save it in the output
        toSave[DIM_VALUE] = r.first;
        IOList::iterator iter = m_outputs.find(d.first);
        if (iter != m_outputs.end())
        {
          iter->second->addExpr(Tuple(toSave), 0);
        }
      }
    }
  }

  ++m_time;
  #endif
}

TaggedConstant
System::operator()(const Tuple& k)
{
  Tuple::const_iterator iditer = k.find(DIM_ID);

  //the system only varies in the id dimension
  if (iditer == k.end())
  {
    return TaggedConstant(Constant(Special(Special::DIMENSION),
                          TYPE_INDEX_SPECIAL), k);
  }
  else
  {
    const u32string& id = iditer->second.value<String>().value();
    auto viter = m_equations.find(id);

    if (viter == m_equations.end())
    {
      return TaggedConstant(Constant(Special(Special::UNDEF),
                            TYPE_INDEX_SPECIAL), k);
    }
    else
    {
      tuple_t kp = k.tuple();
      kp.erase(DIM_ID);
      return (*viter->second)(Tuple(kp));
    }
  }
}

uuid
System::addEquation(const u32string& name, const GuardWS& guard, WS* e)
{
  auto i = m_equations.find(name);
  VariableWS* var = nullptr;

  if (i == m_equations.end())
  {
    var = new VariableWS(name, this);
    m_equations.insert(std::make_pair(name, var));
  }
  else
  {
    var = i->second;
  }

  return var->addEquation(name, guard, e, m_time);
}

WS*
System::translate_expr(const u32string& s)
{
  return m_translator->translate_expr(s);
}

std::list<std::pair<uuid, Parser::ParsedEquation>>
System::translate_and_add_equation_set(const u32string& s)
{
  return m_translator->translate_and_add_equation_set(s);
}

PTEquationVector
System::translate_equation_set(const u32string& s)
{
  return m_translator->translate_equation_set(s);
}

bool
System::parse_header(const u32string& s)
{
  return m_translator->parse_header(s);
}

void
System::loadLibrary(const u32string& s)
{
  m_translator->loadLibrary(s);
}

Parser::Header&
System::header()
{
  return m_translator->header();
}

const Tree::Expr&
System::lastExpression() const
{
  return m_translator->lastExpression();
}

} //namespace TransLucid
