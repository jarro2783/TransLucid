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

#include <tl/builtin_types.hpp>
#include <tl/consthd.hpp>
#include <tl/system.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/translator.hpp>
#include <tl/utility.hpp>
#include <tl/valuehd.hpp>

#include <algorithm>
#include <unordered_map>

namespace TransLucid
{

namespace
{

  enum LineType
  {
    LINE_EQN,
    LINE_ASSIGN,
    LINE_DIM,
    LINE_INFIXL,
    LINE_INFIXR,
    LINE_INFIXN,
    LINE_LIBRARY,
    LINE_POSTFIX,
    LINE_PREFIX
  };

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

std::list<std::pair<uuid, Parser::Equation>>
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

Constant
System::parseLine(Parser::U32Iterator& begin, const Parser::U32Iterator& end)
{
  std::unordered_map<u32string, LineType> lineTypes = 
  {
    {U"eqn", LINE_EQN}, 
    {U"assign", LINE_ASSIGN},
    {U"dim", LINE_DIM},
    {U"infixl", LINE_INFIXL},
    {U"infixr", LINE_INFIXR},
    {U"infixn", LINE_INFIXN},
    {U"library", LINE_LIBRARY},
    {U"prefix", LINE_PREFIX},
    {U"postfix", LINE_POSTFIX}
  };

  u32string firstWord;
  Parser::U32Iterator& current = begin;
  while (current != end && *current != ' ')
  {
    firstWord += *current;
    ++current;
  }

  auto iter = lineTypes.find(firstWord);
  if (iter != lineTypes.end())
  {
    LineType type;
    type = iter->second;

    switch (type)
    {
      case LINE_EQN:
      {
        auto result = m_translator->parseEquation(current, end);
        if (result.first)
        {
          return addEquation(result.second);
        }
        else
        {
        }
      }
      break;

      case LINE_ASSIGN:
      //parse an assignment
      break;

      case LINE_DIM:
      //parse dim
      {
        auto result = m_translator->parseHeaderString(current, end);
        if (result.first)
        {
          addDimension(result.second);
        }
      }
      break;

      case LINE_INFIXL:
      case LINE_INFIXR:
      case LINE_INFIXN:
      //parse binary
      {
        auto result = m_translator->parseHeaderBinary(current, end); 
      }
      break;

      case LINE_LIBRARY:
      //parse library
      {
        auto result = m_translator->parseHeaderString(current, end);
        if (result.first)
        {
          loadLibrary(result.second);
        }
      }
      break;

      case LINE_PREFIX:
      case LINE_POSTFIX:
      //parse unary
      break;
    }
  }
  else
  {
    //invalid keyword
  }

  //return std::make_tuple(
  
}

Constant
System::addEquation(const Parser::Equation& eqn)
{
}

Constant 
System::addDimension(const u32string& dimension)
{
}

} //namespace TransLucid
