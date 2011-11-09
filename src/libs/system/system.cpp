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
//ATL_SYMBOL | [symbol : "s"] = "fnname";;
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

#include <algorithm>
#include <unordered_map>
#include <initializer_list>

#include <tl/builtin_types.hpp>
#include <tl/constws.hpp>
#include <tl/context.hpp>
#include <tl/function_registry.hpp>
#include <tl/output.hpp>
#include <tl/parser-new.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/system.hpp>
#include <tl/translator.hpp>
#include <tl/tree_to_wstree.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/special.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types/workshop.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace
{
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

  Constant
  compile_and_evaluate(const Tree::Expr& expr, System& system)
  {
    TreeToWSTree tows(&system);

    Tree::Expr wsTree = tows.toWSTree(expr);

    WorkshopBuilder compiler(&system);

    std::auto_ptr<WS> ws(compiler.build_workshops(wsTree));

    return (*ws)(system.getDefaultContext());
  }

  //for every context in ctxts that is valid in k,
  //output the result of computation compute to out
  //at the moment we only know how to enumerate ranges, this could
  //become richer as we work out the type system better
  void
  enumerateContextSet
  (
    const Tuple& ctxts, 
    const Tuple& k,
    WS& compute,
    OutputHD* out
  )
  {
    Tuple variance = out->variance();

    //all the ranges in order
    std::vector<Range> limits;

    //the current value of each range dimension
    std::vector<std::pair<dimension_index, mpz_class>> current;

    //the context to evaluate in
    Context evalContext(k);

    //determine which dimensions are ranges
    for (const auto& v : ctxts)
    {
      if (v.second.index() == TYPE_INDEX_RANGE)
      {
        const Range& r = Types::Range::get(v.second);

        if (r.lower() == nullptr || r.upper() == nullptr)
        {
          std::cerr << "Error: infinite bounds in demand, dimension " <<
            v.first << std::endl;
          throw "Infinite bounds in demand";
        }

        limits.push_back(r);
        current.push_back(std::make_pair(v.first, *r.lower()));
      }
      else
      {
        //if not a range then store it permanantly
        evalContext.perturb(v.first, v.second);
      }
    }

    //by doing it this way, even if there is no range, we still evaluate
    //everything once
    while (true)
    {
      ContextPerturber p(evalContext);
      for (const auto& v : current)
      {
        p.perturb(v.first, Types::Intmp::create(v.second));
      }

      //evaluate all the stuff if applicable
      if (tupleApplicable(variance, evalContext))
      {
        out->put(evalContext, compute(evalContext));
      }
      
      //then we increment the counters at the end
      auto limitIter = limits.begin();
      auto currentIter = current.begin();
      while (currentIter != current.end())
      {
        ++currentIter->second;
        if (!limitIter->within(currentIter->second))
        {
          currentIter->second = *limitIter->lower();
          ++currentIter;
          ++limitIter;
        }
        else
        {
          break;
        }
      }
      
      if (currentIter == current.end())
      {
        break;
      }
    }
  }

  class ArgsWorkshop : public WS
  {
    public:
    ArgsWorkshop()
    {
    }

    Constant
    operator()(Context& k)
    {
      //this should always work, but it's a bit hacky

      //head of ##\psi
      Constant hashPsi = k.lookup(DIM_PSI);

      if (hashPsi.index() != TYPE_INDEX_DIMENSION)
      {
        throw "list dimension not a dimension";
      }

      Constant hashHashPsi = k.lookup(get_constant<dimension_index>(hashPsi));

      if (hashHashPsi.index() != TYPE_INDEX_TUPLE)
      {
        throw "list expected, type not a tuple";
      }

      Constant hashPi = k.lookup(DIM_PI);

      if (hashPi.index() != TYPE_INDEX_DIMENSION)
      {
        throw "list dimension not a dimension";
      }

      Constant hashHashPi = k.lookup(get_constant<dimension_index>(hashPi));

      if (hashHashPi.index() != TYPE_INDEX_TUPLE)
      {
        throw "list expected, type not a tuple";
      }

      //hashHashPsi will be a list of workshop objects

      //expr is a workshop object
      Constant expr = listHead(hashHashPsi);

      try
      {

        ContextPerturber p(k,
          {
            {get_constant<dimension_index>(hashPsi), listTail(hashHashPsi)},
            {get_constant<dimension_index>(hashPi), listTail(hashHashPi)}
          }
        );

        p.perturb(Types::Tuple::get(listHead(hashHashPi)));

        WS* w = Types::Workshop::get(expr).ws();

        return (*w)(k);
      
      }
      catch (...)
      {
        std::cerr << "caught exception in args lookup" << std::endl
        << "#pi = " << get_constant<dimension_index>(hashPi) << std::endl
        << "#psi = " << get_constant<dimension_index>(hashPsi) << std::endl;

        throw;
      }

    }
  };
}

namespace detail
{
  class InputHDWS : public WS
  {
    public:
    InputHDWS(const u32string& name, System& system)
    : m_name(name)
    , m_system(system)
    , m_hd(0)
    {
      auto iter = m_system.m_inputHDs.find(m_name);
      if (iter == m_system.m_inputHDs.end())
      {
        //this should never be true, throw if it fails because
        //the system has bombed on us
        throw "InputHDWS: input HD doesn't exist";
      }
      m_hd = iter->second;
    }

    Constant
    operator()(Context& k)
    {
      return m_hd->get(k);
    }

    private:
    u32string m_name;
    System& m_system;
    InputHD* m_hd;
  };

  class LineAdder
  {
    public:

    LineAdder(System& s, bool verbose)
    : m_system(s)
    , m_verbose(verbose)
    {
    }

    typedef Constant result_type;

    Constant
    operator()(const Tree::BinaryOperator& binop)
    {
      return m_system.addBinaryOperator(binop);
    }

    Constant
    operator()(const Tree::UnaryOperator& unop)
    {
      return m_system.addUnaryOperator(unop);
    }

    Constant
    operator()(const Parser::Variable& var)
    {
    }

    Constant
    operator()(const Parser::Assignment& assign)
    {
    }

    #if 0
    Constant
    operator()(const std::pair<Parser::Equation, Parser::DeclType>& eqn)
    {
      if (m_verbose)
      {
        std::cout << Parser::printEquation(eqn.first) << std::endl;
      }
      if (eqn.second == Parser::DECL_DEF)
      {
        return m_system.addEquation(eqn.first);
      }
      else
      {
        return m_system.addAssignment(eqn.first);
      }
    }
    #endif

    Constant
    operator()(const Parser::DimensionDecl& dim)
    {
      return m_system.addDimension(dim.dim);
    }

    Constant
    operator()(const Parser::LibraryDecl& lib)
    {
      //what should this return
      m_system.loadLibrary(lib.lib);
      return Constant();
    }

    Constant
    operator()(const Parser::OutputDecl& out)
    {
      //output hd declaration
      Constant hd = compile_and_evaluate(std::get<3>(out.eqn), m_system);

      if (hd.index() != TYPE_INDEX_OUTHD)
      {
        return Types::Special::create(SP_CONST);
      }

      Constant uuid = m_system.addOutputHyperdaton
      (
        std::get<0>(out.eqn),
        dynamic_cast<OutputHD*>(Types::Hyperdatons::get(hd))
      );

      hd.data.ptr->release();

      if (get<Tree::nil>(&std::get<1>(out.eqn)) == nullptr)
      {
        m_system.addOutputDeclaration(
          std::get<0>(out.eqn), std::get<1>(out.eqn));
      }

      return uuid;

    }

    Constant
    operator()(const Parser::InputDecl& in)
    {
      //input hd declaration
      Constant hd = compile_and_evaluate(std::get<3>(in.eqn), m_system);

      if (hd.index() != TYPE_INDEX_INHD)
      {
        return Types::Special::create(SP_CONST);
      }

      Constant uuid = m_system.addInputHyperdaton
      (
        std::get<0>(in.eqn),
        dynamic_cast<InputHD*>(Types::Hyperdatons::get(hd))
      );

      hd.data.ptr->release();

      if (get<Tree::nil>(&std::get<1>(in.eqn)) == nullptr)
      {
        m_system.addInputDeclaration(std::get<0>(in.eqn), std::get<1>(in.eqn));
      }

      return uuid;
    }

    private:
    System& m_system;
    bool m_verbose;
  };
}

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
    Tree::TupleExpr({{Tree::DimensionExpr(U"symbol"), s}}),
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

template <typename... Args>
void 
addEqn(System& s, Args... args)
{
  s.addEquation(Parser::Equation(
    args...
  ));
}

template <typename Arg>
void
addInitEqn(System& s, const u32string& name, Arg&& e)
{
  s.addEquation(Parser::Equation(
    name,
    Tree::Expr(),
    Tree::Expr(),
    std::forward<Arg>(e)
  ));
}

void
System::init_equations()
{
  //add DIM=false default equation
  addInitEqn(*this,
    U"ID_TYPE",
    U"ID"
  );

  //add PRINT="this type has no printer"
  addInitEqn(*this,
    U"PRINT",
    u32string(U"This type has no printer!")
  );

  //args = case hd(##\psi) of E @ [#\pi <- tl(##\pi), #\psi <- tl(##\psi)]
  //  @ hd(##\pi)
  addEquation(U"args", GuardWS(), new ArgsWorkshop);
}

void 
System::init_dimensions(const std::initializer_list<u32string>& args)
{
  for (auto s : args)
  {
    addDimension(s);
  }
}

System::System()
: m_typeRegistry(m_nextTypeIndex,
  std::vector<std::pair<u32string, type_index>>{
   {U"ustring", TYPE_INDEX_USTRING},
   {U"intmp", TYPE_INDEX_INTMP},
   {U"bool", TYPE_INDEX_BOOL},
   {U"special", TYPE_INDEX_SPECIAL},
   {U"uchar", TYPE_INDEX_UCHAR},
   {U"dim", TYPE_INDEX_DIMENSION},
   {U"type", TYPE_INDEX_TYPE},
   {U"range", TYPE_INDEX_RANGE}
  }
  )
, m_time(0)
, m_uniqueVarIndex(0)
, m_uniqueDimIndex(0)
, m_hiddenDim(-1)
, m_debug(false)
{
  //create the obj, const and fun ids

  setDefaultContext();

  //m_translator = new Translator(*this);
  m_parser = new Parser::Parser(*this);

  //these are for the lexer
  init_dimensions
  (
    {
      U"time",
      U"name",
      U"symbol",
      U"fnname",
      U"arg0",
      U"arg1",
      U"arg2",
      U"typename",
      U"text"
    }
  );

  init_equations();

  init_builtin_types(*this);
}

System::~System()
{
  //delete m_translator;
  delete m_parser;

  //delete the equations
  for (auto& v : m_assignments)
  {
    delete v.second;
  }

  for (auto& v : m_equations)
  {
    delete v.second;
  }
}

void
System::go()
{
  for (const auto& ident : m_assignments)
  {
    auto hd = m_outputHDs.find(ident.first);

    if (hd == m_outputHDs.end())
    {
      std::cerr << "warning: output hyperdaton \"" << ident.first
        << "\" doesn't exist" << std::endl;
      continue;
    }

    Context theContext = m_defaultk;

    //this needs to be way better
    //for a start: only look at demands for the current time
    auto equations = ident.second->equations();
    for (auto& assign : equations)
    {
      const Tuple& constraint = m_outputHDDecls.find(ident.first)->second;
      const GuardWS& guard = assign.second.validContext();

      Tuple ctxts = guard.evaluate(theContext);

      ContextPerturber p(theContext, constraint);

      //the demand could have ranges, so we need to enumerate them
      enumerateContextSet(ctxts, theContext, assign.second, hd->second);
    }
  }

  //commit all of the hyperdatons
  for (auto outHD : m_outputHDs)
  {
    outHD.second->commit();
  }

  ++m_time;
  setDefaultContext();
}

uuid
System::addEquation(const u32string& name, const GuardWS& guard, WS* e)
{
  return addDeclInternal(name, guard, e, m_equations);
}

//bool
//System::parse_header(const u32string& s)
//{
  //return m_translator->parse_header(s);
//}

void
System::loadLibrary(const u32string& s)
{
  //m_translator->loadLibrary(s);
}

Parser::Header&
System::header()
{
  //return m_translator->header();
}

const Tree::Expr&
System::lastExpression() const
{
  //return m_translator->lastExpression();
}

Constant
System::parseLine
(
  Parser::StreamPosIterator& begin, 
  const Parser::StreamPosIterator& end,
  bool verbose,
  bool debug
)
{
  //TODO this is a hack, implement debug and verbose properly
  //some sort of output object might be good too
  m_debug = debug;

  Parser::Line line;

  Parser::LexerIterator lexit(begin, end, m_defaultk, lookupIdentifiers());
  Parser::LexerIterator lexend = lexit.makeEnd();
  bool success = m_parser->parse_line(lexit, lexend, line);

  if (success)
  {
    detail::LineAdder adder(*this, verbose);
    Constant c = apply_visitor(adder, line);

    //this is the best way that we know for now, fix up the default context for
    //the L_in dims that were added
    setDefaultContext();

    return c;
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

  uuid u = generate_uuid();

  m_unop_uuids.insert(std::make_pair(u,
    UnaryUUIDs
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

  uuid u = generate_uuid();

  m_binop_uuids.insert(std::make_pair(u, 
    BinaryUUIDs
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
  TreeToWSTree tows(this);

  //simplify, turn into workshops
  Tree::Expr guard   = toWSTreePlusExtras(std::get<1>(eqn), tows);
  Tree::Expr boolean = toWSTreePlusExtras(std::get<2>(eqn), tows);
  Tree::Expr expr    = toWSTreePlusExtras(std::get<3>(eqn), tows);

  if (m_debug)
  {
    //print everything
    std::cout << "The rewritten expr" << std::endl; 
    std::cout << 
      Parser::printEquation
      (std::make_tuple(std::get<0>(eqn), guard, boolean, expr)) 
      << std::endl;

    for (const auto& e : tows.newVars())
    {
      std::cout << Parser::printEquation(e) << std::endl;
    }
  }

  WorkshopBuilder compile(this);

  uuid u = addDeclInternal(
    std::get<0>(eqn),
    GuardWS(compile.build_workshops(guard),
      compile.build_workshops(boolean)),
    compile.build_workshops(expr),
    declarations
  );

  //add all the new equations
  for (const auto& e : tows.newVars())
  {
    addDeclInternal(
      std::get<0>(e),
      GuardWS(compile.build_workshops(std::get<1>(e)),
        compile.build_workshops(std::get<2>(e))),
      compile.build_workshops(std::get<3>(e)),
      declarations
    );
  }

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

std::pair<bool, Tree::Expr>
System::parseExpression(Parser::U32Iterator& iter)
{
  //return m_translator->parseExpr(iter, m_defaultk);
}

Constant
System::addOutputHyperdaton
(
  const u32string& name,
  OutputHD* hd
)
{
  //make sure the name isn't already there
  auto i = m_outputHDs.find(name);

  if (i == m_outputHDs.end())
  {
    m_outputHDs.insert(std::make_pair(name, hd));
    uuid u = generate_uuid();
    m_outputUUIDs.insert(std::make_pair(u, name));

    //add the constraint
    m_outputHDDecls.insert(std::make_pair(name, hd->variance()));

    return Types::UUID::create(u);
  }
  else
  {
    return Types::Special::create(SP_MULTIDEF); 
  }
}

void
System::setDefaultContext()
{
  m_defaultk.reset();

  m_defaultk.perturb(Tuple
  (
    tuple_t
      {
        {DIM_TIME, Types::Intmp::create(m_time)}
      }
  ));

  //set the L_in dimensions to zero
  for (auto d : m_Lin)
  {
    m_defaultk.perturb(d, Types::Intmp::create(0));
  }

  //set the function list dimensions to nil
  for (auto d : m_fnLists)
  {
    m_defaultk.perturb(d, Types::Tuple::create
    (
      tuple_t
      {
        {DIM_TYPE, Types::String::create(U"list")},
        {DIM_CONS, Types::String::create(U"nil")}
      }
    )
    );
  }
}

template <typename T>
void
System::addHDDecl
(
  const u32string& name,
  const Tree::Expr& guard,
  T& decls
)
{
  //compile and evaluate guard
  Constant c = compile_and_evaluate(guard, *this);
  
  //add as new constraint to output HD
  if (c.index() == TYPE_INDEX_TUPLE)
  {
    auto iter = decls.find(name);

    if (iter != decls.end())
    {
      const Tuple& t = Types::Tuple::get(c);
      //only update if the tuple is more specific than or the same as the
      //existing one
      if (t == iter->second || tupleRefines(t, iter->second))
      {
        iter->second = t;
      }
    }
  }
}

void
System::addOutputDeclaration
(
  const u32string& name,
  const Tree::Expr& guard
)
{
  addHDDecl(name, guard, m_outputHDDecls);
}

void
System::addInputDeclaration
(
  const u32string& name,
  const Tree::Expr& guard
)
{
  addHDDecl(name, guard, m_inputHDDecls);
}

Constant
System::addInputHyperdaton
(
  const u32string& name,
  InputHD* hd
)
{
  //make sure the name isn't already there
  auto i = m_inputHDs.find(name);

  if (i == m_inputHDs.end())
  {
    m_inputHDs.insert(std::make_pair(name, hd));
    uuid u = generate_uuid();
    //m_outputUUIDs.insert(std::make_pair(u, name));

    //add the constraint
    //m_outputHDDecls.insert(std::make_pair(name, hd->variance()));

    //the variance becomes the guard, this guarantees that requests are for
    //a valid index
    auto ws = new detail::InputHDWS(name, *this);
    addEquation(name, GuardWS(hd->variance()), ws);

    return Types::UUID::create(u);
  }
  else
  {
    return Types::Special::create(SP_MULTIDEF); 
  }
}

Tree::Expr
System::toWSTreePlusExtras(const Tree::Expr& e, TreeToWSTree& tows)
{
  Tree::Expr wstree = tows.toWSTree(e);

  //The L_in dims are set to 0 in the default context
  m_Lin.insert(m_Lin.end(), tows.getLin().begin(), tows.getLin().end());

  m_fnLists.insert(m_fnLists.end(), 
    tows.getAllScopeArgs().begin(), tows.getAllScopeArgs().end());

  m_fnLists.insert(m_fnLists.end(), 
    tows.getAllScopeOdometer().begin(), tows.getAllScopeOdometer().end());

  return wstree;
}

} //namespace TransLucid
