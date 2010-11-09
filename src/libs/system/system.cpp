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

    TaggedConstant
    operator()(const Tuple& k)
    {
      return TaggedConstant(Constant(Intmp(m_index++), TYPE_INDEX_INTMP), k);
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

  class DimensionsTypedHD : public HD
  {
    public:

    DimensionsTypedHD(DimensionTranslator& d)
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
}

template <typename T>
HD*
SystemHD::buildConstantHD(size_t index)
{
  HD* h = new T(this);

  tuple_t guard =
  {
    {
      DIM_TYPE,
      Constant(String(T::name), TYPE_INDEX_USTRING)
    }
  };

  tuple_t k =
  {
    {
      DIM_VALID_GUARD,
      Constant(Guard(GuardHD(Tuple(guard))), TYPE_INDEX_GUARD)
    }
  };

  //sets the following

  //CONST | [type : ustring<name>]
  addToVariableActual(U"CONST", Tuple(k), h);

  //TYPE_INDEX | [type : ustring<name>] = index;;
  addToVariableActual
  (
    U"TYPE_INDEX", 
    Tuple(k), 
    new Hyperdatons::IntmpConstHD(index)
  );
  return h;
}

void
SystemHD::init_types()
{
  BOOST_FOREACH(auto v, builtin_name_to_index)
  {
    addToVariableActual(v.first, Tuple(),
                        new Hyperdatons::TypeConstHD(v.second));
  }
}

SystemHD::SystemHD()
: VariableHD(U"", this),
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
  addToVariableActual(U"DIMENSION_NAMED_INDEX", Tuple(),
                      new DimensionsStringHD(m_dimTranslator));
  addToVariableActual(U"DIMENSION_VALUE_INDEX", Tuple(),
                      new DimensionsTypedHD(m_dimTranslator));
  addToVariableActual
  (
    U"_unique", Tuple(),
    new UniqueHD(std::max
    (
      static_cast<int>(TYPE_INDEX_LAST),
      static_cast<int>(DIM_INDEX_LAST)
    ))
  );

  //add this
  addToVariableActual(U"this", Tuple(), this);

  //add variables for all the types
  //std::vector<ustring_t> typeNames = {"intmp", "uchar"};
  init_types();

  //build the constant creators
  buildConstantHD<Hyperdatons::BoolHD>(TYPE_INDEX_BOOL);
  buildConstantHD<Hyperdatons::UCharHD>(TYPE_INDEX_UCHAR);
  HD* intmpHD = buildConstantHD<Hyperdatons::IntmpHD>(TYPE_INDEX_INTMP);
  buildConstantHD<Hyperdatons::UStringHD>(TYPE_INDEX_USTRING);

  //set this as the default int too
  addToVariableActual(U"DEFAULTINT", Tuple(), intmpHD);
}

void
SystemHD::addOutput(const IOList& output)
{
  m_outputs.insert(output.begin(), output.end());
}

void
SystemHD::addInput(const IOList& input)
{
  m_inputs.insert(input.begin(), input.end());

  //add all the inputs as equations
  #warning clear up semantics here, this only works if the names do not
  #warning clash and there is only one equation per name, otherwise trouble
  m_variables.insert(input.begin(), input.end());
}

void
SystemHD::addDemand(const u32string& id, const GuardHD& guard)
{
  m_demands.insert(std::make_pair(id, guard));
}

void
SystemHD::tick()
{

  tuple_t current;
  current[DIM_TIME] = Constant(Intmp(m_time), TYPE_INDEX_INTMP);

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
}

} //namespace TransLucid
