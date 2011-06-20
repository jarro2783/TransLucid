/* The System.
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

//--------- Header Equations -------------
//For dimensions:
//DIM | [name : "dimension"] = true;;
//
//For operations:
//OPTYPE | [symbol : "s"] = "{PREFIX, POSTFIX, BINARY}";;
//ATL_SYMBOL | [symbol : "s"] = "opname";;
//
//For binary:
//ASSOC | [symbol : "s"] = "{LEFT, RIGHT, NON}";;
//PREC  | [symbol : "s"] = N;;

//---- Adding Equations / Assignments ----
//As these are both almost identical, but are stored in a different variable,
//the code for adding these has been written only once.
//There are two types of functions:
//add*(Parser::Equation) and
//add*(name, guard, varws)
//The common code has been put in:
//addDeclInternal
//with both overloads. The former translates the trees and then simply calls
//the latter. They both take a reference to the data structure to store these
//in.
//The specific functions addEquation and addAssignment are both wrappers for
//the appropriate addDeclInternal, they pass m_equations and m_assignments
//respectively.


#include <tl/builtin_types.hpp>
#include <tl/consthd.hpp>
#include <tl/function_registry.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/system.hpp>
#include <tl/translator.hpp>
#include <tl/tree_to_wstree.hpp>
#include <tl/types/special.hpp>
#include <tl/types/uuid.hpp>
#include <tl/utility.hpp>

#include <algorithm>
#include <unordered_map>
#include <initializer_list>

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

  bool
  hasSpecial(const std::initializer_list<Constant>& c)
  {
    for(auto v : c)
    {
      if (v.index() == TYPE_INDEX_SPECIAL)
      {
        return true;
      }
    }
    return false;
  }

  #if 0
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
  #endif
}
//
//adds eqn | [symbol : "s"] = value;;
template <typename T>
Constant
System::addSymbolInfo
(
  const u32string& eqn, 
  const u32string& s, 
  const T& value
)
{
  return addEquation(Parser::Equation(
    eqn,
    Tree::TupleExpr({{U"symbol", s}}),
    Tree::Expr(),
    value
  ));
}


template <typename T>
WS*
System::buildConstantWS(size_t index)
{
  WS* h = new T(this);

  #if 0
  tuple_t guard =
  {
    {
      DIM_TYPE,
      Constant(String(T::name), TYPE_INDEX_USTRING)
    }
  };
  #endif

  //sets the following

  //CONST | [type : ustring<name>]
  //addEquation(U"CONST", GuardWS(Tuple(guard)), h);

  addEquation(Parser::Equation
    (
      U"LITERAL", 
      Tree::TupleExpr(Tree::TupleExpr::TuplePairs{{U"DIM_TYPE", T::name}}), 
      Tree::Expr(),
      h
    )
  );

  #if 0
  //TYPE_INDEX | [type : ustring<name>] = index;;
  addEquation
  (
    U"TYPE_INDEX", 
    GuardWS(Tuple(guard)), 
    new Hyperdatons::IntmpConstWS(index)
  );
  #endif

  //TODO: sort out the default literals

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

  //LITERAL | [type : "t", text : ustring] = "hostfun"!(#text)
}

System::System()
: m_typeRegistry(m_nextTypeIndex)
, m_time(0)
, builtin_name_to_index
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
  #if 0
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
  #endif

  //add this
  //addEquation(U"this", GuardWS(), this);

  //add variables for all the types
  //std::vector<ustring_t> typeNames = {"intmp", "uchar"};
  init_types();

  //build the constant creators
  //buildConstantWS<Hyperdatons::BoolWS>(TYPE_INDEX_BOOL);
  //buildConstantWS<Hyperdatons::UCharWS>(TYPE_INDEX_UCHAR);
  //WS* intmpWS = buildConstantWS<Hyperdatons::IntmpWS>(TYPE_INDEX_INTMP);
  //buildConstantWS<Hyperdatons::UStringWS>(TYPE_INDEX_USTRING);

  //set this as the default int too
  //addEquation(U"DEFAULTINT", GuardWS(), intmpWS);

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

uuid
System::addEquation(const u32string& name, const GuardWS& guard, WS* e)
{
  return addDeclInternal(name, guard, e, m_equations);
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

static Tree::UnaryOperator
makeUnaryOp(LineType type, const Parser::UnopHeader& op)
{
  Tree::UnaryType ut = 
    type == LINE_POSTFIX ? Tree::UNARY_POSTFIX : Tree::UNARY_PREFIX;
  return Tree::UnaryOperator(std::get<1>(op), std::get<0>(op), ut);
}

static Tree::BinaryOperator
makeBinaryOp(LineType type, const Parser::BinopHeader& op)
{
  Tree::InfixAssoc a;
  switch (type)
  {
    case LINE_INFIXL:
    a = Tree::ASSOC_LEFT;
    break;

    case LINE_INFIXR:
    a = Tree::ASSOC_RIGHT;
    break;

    case LINE_INFIXN:
    a = Tree::ASSOC_NON;
    break;

    default:
    throw __FILE__ ":" STRING_(__LINE__) ": oops";
    break;
  }

  return Tree::BinaryOperator(a, 
    std::get<1>(op), std::get<0>(op), std::get<2>(op));
}

Constant
System::parseLine(Parser::U32Iterator& begin, const Parser::U32Iterator& end)
{
  std::cerr << "parse line..." << std::endl;
  Parser::U32Iterator& current = begin;

  //skip over spaces
  char32_t c = *current;
  while (current != end)
  {
    c = *current;
    if (!(c == ' ' || c == '\t' || c == '\n'))
    {
      break;
    }
    ++current;
  }

  //read the first word
  u32string firstWord;
  while (current != end && *current != ' ')
  {
    firstWord += *current;
    ++current;
  }

  std::cerr << "parsing thing of type " << firstWord << std::endl;
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
        if (result.first && result.second.second == Parser::DECL_DEF)
        {
          return addEquation(result.second.first);
        }
        else
        {
        }
      }
      break;

      case LINE_ASSIGN:
      //parse an assignment
      {
        auto result = m_translator->parseEquation(current, end);
        if (result.first && result.second.second == Parser::DECL_ASSIGN)
        {
          return addAssignment(result.second.first);
        }
      }
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
        if (result.first)
        {
          addBinaryOperator(makeBinaryOp(type, result.second));
        }
      }
      break;

      case LINE_LIBRARY:
      //parse library
      {
        auto result = m_translator->parseHeaderString(current, end);
        if (result.first)
        {
          //what should loading a library return?
          loadLibrary(result.second);
        }
      }
      break;

      case LINE_PREFIX:
      case LINE_POSTFIX:
      //parse unary
      {
        auto result = m_translator->parseHeaderUnary(current, end);
        if (result.first)
        {
          return addUnaryOperator(makeUnaryOp(type, result.second));
        }
      }
      break;
    }
  }
  else
  {
    //invalid keyword
  }

  return Types::Special::create(SP_CONST); 
}

Constant
System::addEquation(const Parser::Equation& eqn)
{
  return addDeclInternal(eqn, m_equations);
}

Constant 
System::addDimension(const u32string& dimension)
{
  std::cerr << "adding dimension: " << dimension << std::endl;
  //add equation DIM | [name : "dimension"] = true
  Constant c = addEquation(Parser::Equation(
    U"DIM", 
    Tree::TupleExpr({{Tree::DimensionExpr(U"name"), dimension}}), 
    Tree::Expr(), 
    true
  ));

  //add to the set of dimensions
  if (c.index() != TYPE_INDEX_SPECIAL)
  {
    //TODO make the uuid type and then this is where we extract it
    m_dimension_uuids.insert(Types::UUID::get(c));
  }

  return c;
}

Constant
System::addUnaryOperator(const Tree::UnaryOperator& op)
{
  u32string typeName;

  Constant a = addATLSymbol(op.symbol, op.op);
  if (op.type == Tree::UNARY_PREFIX)
  {
    typeName = U"PREFIX";
  }
  else
  {
    typeName = U"POSTFIX";
  }

  Constant t = addOpType(op.symbol, typeName);

  if (hasSpecial({a, t}))
  {
    return Types::Special::create(SP_CONST);
  }

  uuid u = m_uuid_generator();

  m_unop_uuids.insert(std::make_pair(u,
    UnaryHashes
    (
      Types::UUID::get(a),
      Types::UUID::get(t)
    )
  ));

  return Types::UUID::create(u);
}

Constant
System::addBinaryOperator(const Tree::BinaryOperator& op)
{
  u32string assocName;

  switch(op.assoc)
  {
    case Tree::ASSOC_LEFT:
    assocName = U"LEFT";
    break;

    case Tree::ASSOC_RIGHT:
    assocName = U"RIGHT";
    break;

    case Tree::ASSOC_NON:
    assocName = U"NON";
    break;
    
    default:
    //if this exception is raised then we added different associativity
    //operators but forgot to take care of them here
    throw __FILE__ ":" STRING_(__LINE__) ": oops";
    break;
  }

  Constant t = addOpType(op.symbol, U"BINARY");
  Constant s = addATLSymbol(op.symbol, op.op);
  Constant a = addAssoc(op.symbol, assocName);
  Constant p = addPrecedence(op.symbol, op.precedence);

  if (hasSpecial({t, s, a, p}))
  {
    return Types::Special::create(SP_CONST);
  }

  uuid u = m_uuid_generator();

  m_binop_uuids.insert(std::make_pair(u, 
    BinaryHashes
    (
      Types::UUID::get(t),
      Types::UUID::get(s),
      Types::UUID::get(a),
      Types::UUID::get(p)
    )
  ));

  return Types::UUID::create(u);
}

Constant 
System::addOpType(const u32string& symbol, const u32string& type)
{
  return addSymbolInfo(U"OPTYPE", symbol, type);
}

Constant
System::addATLSymbol(const u32string& symbol, const u32string& op)
{
  return addSymbolInfo(U"ATL_SYMBOL", symbol, op);
}

Constant
System::addAssoc(const u32string& symbol, const u32string assoc)
{
  return addSymbolInfo(U"ASSOC", symbol, assoc);
}

Constant
System::addPrecedence(const u32string& symbol, const mpz_class& precedence)
{
  return addSymbolInfo(U"PREC", symbol, precedence);
}

uuid
System::addDeclInternal
(
  const u32string& name, 
  const GuardWS& guard, WS* e,
  DefinitionMap& declarations
)
{
  auto i = declarations.find(name);
  VariableWS* var = nullptr;

  if (i == declarations.end())
  {
    var = new VariableWS(name);
    declarations.insert(std::make_pair(name, var));
  }
  else
  {
    var = i->second;
  }

  return var->addEquation(name, guard, e, m_time);
}

Constant
System::addDeclInternal
(
  const Parser::Equation& eqn, 
  DefinitionMap& declarations
)
{
  //simplify, turn into workshops
  Tree::Expr guard   = toWSTree(std::get<1>(eqn));
  Tree::Expr boolean = toWSTree(std::get<2>(eqn));
  Tree::Expr expr    = toWSTree(std::get<3>(eqn));

  ExprCompiler compile(this);

  uuid u = addDeclInternal(
    std::get<0>(eqn),
    GuardWS(compile.compile_for_equation(guard),
      compile.compile_for_equation(boolean)),
    compile.compile_for_equation(expr),
    declarations
  );

  return Types::UUID::create(u);
}

Constant
System::addAssignment(const Parser::Equation& eqn)
{
  return addDeclInternal(eqn, m_assignments);
}

type_index
System::getTypeIndex(const u32string& name)
{
  return m_typeRegistry(name);
}

dimension_index
System::getDimensionIndex(const u32string& name)
{
  return m_dimTranslator.lookup(name);
}

dimension_index
System::getDimensionIndex(const Constant& c)
{
  return m_dimTranslator.lookup(c);
}

bool 
System::parseInstant
(
  Parser::U32Iterator& begin,
  const Parser::U32Iterator& end
)
{
  return m_translator->parseInstant(begin, end,
  [](const Parser::Instant& i) -> void 
  {
    std::cerr << "end of instant" << std::endl;
  }
  );
}

} //namespace TransLucid
