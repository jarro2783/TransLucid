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

#include <tl/interpreter.hpp>
#include <tl/builtin_types.hpp>
#include <tl/utility.hpp>
#include <tl/consthd.hpp>

namespace TransLucid
{

namespace
{

  class UniqueHD : public HD
  {
    public:

    UniqueHD(int start)
    : m_index(start)
    {
    }

    TaggedValue
    operator()(const Tuple& k)
    {
      return TaggedValue(TypedValue(Intmp(m_index++), TYPE_INDEX_INTMP), k);
    }

    private:
    mpz_class m_index;
  };

  class DimensionsStringHD : public HD
  {
    public:

    DimensionsStringHD(DimensionTranslator& d)
    : m_d(d)
    {}

    TaggedValue
    operator()(const Tuple& k)
    {
      Tuple::const_iterator iter = k.find(DIM_TEXT);
      if (iter == k.end())
      {
        throw "called dim lookup without text dimension";
      }
      return TaggedValue(TypedValue(Intmp(
        m_d.lookup(iter->second.value<String>().value())),
                   TYPE_INDEX_INTMP), k);
    }

    private:
    DimensionTranslator& m_d;
  };

  class DimensionsTypedHD : public HD
  {
    public:

    DimensionsTypedHD(DimensionTranslator& d)
    : m_d(d)
    {}

    TaggedValue
    operator()(const Tuple& k)
    {
      Tuple::const_iterator iter = k.find(DIM_VALUE);
      if (iter == k.end())
      {
        throw "called dim lookup without value dimension";
      }
      return TaggedValue(TypedValue(Intmp(
        m_d.lookup(iter->second)), TYPE_INDEX_INTMP), k);
    }

    private:
    DimensionTranslator& m_d;
  };
}

template <typename T>
HD*
Interpreter::buildConstantHD(size_t index)
{
  HD* h = new T(this);

  tuple_t k;
  Tuple empty;
  //k[m_dimTranslator.lookup("id")] =
  //  TypedValue(String("CONST"), m_typeRegistry.indexString());
  k[DIM_TYPE] = TypedValue(String(T::name), TYPE_INDEX_USTRING);
  addToVariableActual(U"CONST", Tuple(k), h);
  addToVariableActual(U"TYPEINDEX", empty, new Hyperdatons::IntmpConst(index));
  return h;
}

void
Interpreter::init_types()
{
  BOOST_FOREACH(auto v, builtin_name_to_index)
  {
    addToVariableActual(v.first, Tuple(), new Hyperdatons::TypeConst(v.second));
  }
}

Interpreter::Interpreter()
: Variable(U"", this),
  m_time(0),
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
  addToVariableActual(U"DIMENSION_INDEX", Tuple(),
                      new DimensionsStringHD(m_dimTranslator));
  addToVariableActual(U"DIMENSION_TYPED_INDEX", Tuple(),
                      new DimensionsTypedHD(m_dimTranslator));
  addToVariableActual(U"_unique", Tuple(),
                      new UniqueHD(RESERVED_INDEX_LAST));

  //add variables for all the types
  //std::vector<ustring_t> typeNames = {"intmp", "uchar"};
  init_types();

  //build the constant creators
  buildConstantHD<Hyperdatons::UChar>(TYPE_INDEX_UCHAR);
  HD* intmpHD = buildConstantHD<Hyperdatons::Intmp>(TYPE_INDEX_INTMP);
  buildConstantHD<Hyperdatons::UString>(TYPE_INDEX_USTRING);

  //set this as the default int too
  addToVariableActual(U"DEFAULTINT", Tuple(), intmpHD);
}

void
Interpreter::addOutput(const IOList& output)
{
  m_outputs.insert(output.begin(), output.end());
}

void
Interpreter::addInput(const IOList& input)
{
  m_inputs.insert(input.begin(), input.end());

  //add all the inputs as equations
  #warning clear up semantics here, this only works if the names do not
  #warning clash and there is only one equation per name, otherwise trouble
  m_variables.insert(input.begin(), input.end());
}

void
Interpreter::addDemand(const u32string& id, const EquationGuard& guard)
{
  m_demands.insert(std::make_pair(id, guard));
}

void
Interpreter::tick()
{

  tuple_t current;
  current[DIM_TIME] = TypedValue(Intmp(m_time), TYPE_INDEX_INTMP);

  tuple_t toSave = current;

  Tuple k(current);

  //run the applicable demands

  BOOST_FOREACH(const DemandStore::value_type& d, m_demands)
  {
    Tuple t = d.second.evaluate(Tuple());
    if (tupleApplicable(t, k))
    {
      IOList::const_iterator iter = m_variables.find(d.first);
      if (iter != m_variables.end())
      {
        //evaluate the demand
        TaggedValue r = (*iter->second)(k);
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
}

} //namespace TransLucid
