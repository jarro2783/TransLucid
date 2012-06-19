/* The System.
   Copyright (C) 2009--2012 Jarryd Beck

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <initializer_list>
#include <sstream>
#include <unordered_map>

#include <signal.h>
#include <unistd.h>

#include <tl/assignment.hpp>
#include <tl/basefun.hpp>
#include <tl/builtin_types.hpp>
#include <tl/cache.hpp>
#include <tl/constws.hpp>
#include <tl/context.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/function_registry.hpp>
#include <tl/opdef.hpp>
#include <tl/output.hpp>
#include <tl/parser.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/semantic_transform.hpp>
#include <tl/tree_rewriter.hpp>
#include <tl/tree_to_wstree.hpp>
#include <tl/tree_printer.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/function.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/special.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/uuid.hpp>
#include <tl/types/intension.hpp>
#include <tl/workshop_builder.hpp>
#include <tl/types_util.hpp>
#include <tl/utility.hpp>

#include "tl/gettext_internal.h"

namespace TransLucid
{

namespace
{

  std::set<u32string> cacheExclude
  {
    //U"upon",
    //U"merge",
    //U"wvr",
    //U"fby"
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

  Constant
  compile_and_evaluate(const Tree::Expr& expr, System& system)
  {
    Tree::Expr transformed = system.fixupTreeAndAdd(expr);

    WorkshopBuilder compiler(&system);

    std::auto_ptr<WS> ws(compiler.build_workshops(transformed));

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
    Context& k,
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

      //is the demand valid for the hyperdaton, and is the demand valid for
      //the current context
      if (tupleApplicable(variance, evalContext) && k <= evalContext)
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

  class MakeErrorWS : public WS
  {
    public:
    Constant
    operator()(Context& kappa, Context& delta)
    {
      return operator()(kappa);
    }

    Constant
    operator()(Context& k)
    {
      return Constant();
    }
  };

  template <typename Iterator>
  class GenericObject : public SystemObject
  {
    public:

    bool
    del(uuid id, int time)
    {
      return m_object->del(id, time);
    }

    bool
    repl(uuid id, int time, const Parser::Line& line)
    {
      return m_object->repl(id, time, line);
    }

    private:
    Iterator m_object;

    public:
    GenericObject(decltype(m_object) object)
    : m_object(object)
    {}
  };

  template <typename Object>
  std::shared_ptr<GenericObject<Object>>
  make_generic_object(Object&& o)
  {
    return std::make_shared<GenericObject<Object>>(std::forward<Object>(o));
  }
}

GettextInit System::m_gettext;

GettextInit::GettextInit()
{
  bindtextdomain(TRANSLATE_DOMAIN, LOCALEDIR);
}

namespace detail
{
  class InputHDWS : public WS
  {
    public:
    InputHDWS(const u32string& name, System& system)
    : m_name(name)
    , m_system(system)
    , m_hd(nullptr)
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

    Constant
    operator()(Context& kappa, Context& delta)
    {
      //check that the variance is in the delta
      const auto& variance = m_hd->variance();

      std::vector<dimension_index> needs;

      for (const auto& d : variance)
      {
        if (!delta.has_entry(d.first))
        {
          needs.push_back(d.first);
        }
      }

      if (needs.size() != 0)
      {
        return Types::Demand::create(needs);
      }

      return operator()(kappa);
    }

    private:
    u32string m_name;
    System& m_system;
    InputHD* m_hd;
  };
}

//private template  functions go at the top, but after the local
//definitions

template <typename Input>
Constant
System::addVariableDeclInternal
(
  const u32string& name,
  Input&& decl
)
{
  uuid u = generate_uuid();

  auto equationIter = m_variables.find(name);

  std::shared_ptr<VariableWS> var;
  if (equationIter == m_variables.end())
  {
    var = std::make_shared<VariableWS>(name, *this);
    auto variter = m_variables.insert({name, var});

    //create a new VariableObject to manage the uuid
    m_objects.insert({u, make_generic_object(variter.first->second)});
  }
  else
  {
    var = equationIter->second;
  }
  var->addEquation(u, std::forward<Input>(decl), m_time);

  m_identifiers.insert({name, var});

  return Types::UUID::create(u);
}

template <typename Input>
Constant
System::addFunDeclInternal
(
  const u32string& name,
  Input&& decl
)
{
  uuid u = generate_uuid();

  auto funIter = m_functions.find(name);

  std::shared_ptr<FunctionWS> fun;
  if (funIter == m_functions.end())
  {
    fun = std::make_shared<FunctionWS>(name, *this);
    auto funAddIter = m_functions.insert({name, fun});

    m_objects.insert({u, make_generic_object(funAddIter.first->second)});
  }
  else
  {
    fun = funIter->second;
  }

  fun->addEquation(u, decl, m_time);

  m_identifiers.insert({name, fun});

  return Types::UUID::create(u);
}

template <typename Input>
Constant
System::addOpDeclInternal
(
  const u32string& name,
  Input&& decl
)
{
  uuid u = generate_uuid();

  auto opident = m_identifiers.find(U"operator");

  if (opident == m_identifiers.end())
  {
    m_operators = std::make_shared<OpDefWS>(*this);
    m_identifiers.insert({U"operator", m_operators});
  }

  m_operators->addEquation(u, decl, m_time);

  m_objects.insert(
  {
    u, 
    make_generic_object(m_operators)
  });

  return Types::UUID::create(u);
}

template <typename Input>
Constant
System::addConstructorInternal
(
  const u32string& name,
  Input&& decl
)
{
  uuid u = generate_uuid();

  //this name cannot already exist
  auto consIter = m_constructors.find(name);

  if (consIter != m_constructors.end())
  {
    return Types::Special::create(SP_ERROR);
  }

  consIter = m_constructors.insert({name, std::make_shared<ConsDefWS>(*this)})
    .first;

  consIter->second->addEquation(u, std::forward<Input>(decl), m_time);

  m_identifiers.insert({name, consIter->second});

  return Types::UUID::create(u);
}

template <typename... Args>
void 
addEqn(System& s, Args... args)
{
  s.addEquation(Parser::Equation(
    args...
  ));
}

void
addDecl(System& s, const u32string& name, const u32string& value)
{
  s.addEquation(Parser::Equation(
    U"ID_TYPE",
    Tree::TupleExpr(
        {{Tree::DimensionExpr(DIM_ARG0), name}}),
    Tree::Expr(),
    value
    ));
}

void 
System::init_dimensions(const std::initializer_list<u32string>& args)
{
  for (auto s : args)
  {
    addDimension(s);
  }
}

//TODO this type registry stuff should go somewhere that is easier to find
System::System(bool cached)
: m_cached(cached),
  m_cacheEnabled(cached),
  m_nextTypeIndex(-1),
  m_typeRegistry(m_nextTypeIndex,
  std::vector<std::pair<u32string, type_index>>{
   {U"error", TYPE_INDEX_ERROR},
   {U"ustring", TYPE_INDEX_USTRING},
   {U"intmp", TYPE_INDEX_INTMP},
   {U"floatmp", TYPE_INDEX_FLOATMP},
   {U"bool", TYPE_INDEX_BOOL},
   {U"special", TYPE_INDEX_SPECIAL},
   {U"uchar", TYPE_INDEX_UCHAR},
   {U"dim", TYPE_INDEX_DIMENSION},
   {U"tuple", TYPE_INDEX_TUPLE},
   {U"typetype", TYPE_INDEX_TYPE},
   {U"range", TYPE_INDEX_RANGE},
   {U"lambda", TYPE_INDEX_VALUE_FUNCTION},
   {U"phi", TYPE_INDEX_NAME_FUNCTION},
   {U"uuid", TYPE_INDEX_UUID},
   {U"demand", TYPE_INDEX_DEMAND},
   {U"calc", TYPE_INDEX_CALC},
   {U"basefun", TYPE_INDEX_BASE_FUNCTION},
   {U"union", TYPE_INDEX_UNION},
   {U"intension", TYPE_INDEX_INTENSION}
  }
  )
, m_time(0)
, m_uniqueVarIndex(0)
, m_uniqueDimIndex(0)
, m_hiddenDim(-1)
, m_whereCounter(0)
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
      U"priority",
      U"time",
      U"name",
      U"symbol",
      U"fnname",
      U"typename",
      U"text",
      U"cons",
      U"type"
    }
  );

  init_builtin_types(*this);
}

System::~System()
{
  //delete m_translator;
  delete m_parser;

  //clean up cache nodes
  for (auto& c : m_cachedVars)
  {
    delete c.second;
  }
}

void
System::go()
{
  setDefaultContext();
  for (auto& assign : m_assignments)
  {
    assign.second->evaluate(*this, m_defaultk);
  }

  //collect some garbage
  for (auto& cached : m_cachedVars)
  {
    #if 0
    std::cerr << cached.first << ": " 
              << cached.second->getCache().hits() << " hits, and "
              << cached.second->getCache().misses() << " misses"
              << std::endl;
    #endif
    cached.second->garbageCollect();
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
  return addDeclInternal(name, guard, e, m_equations, m_equationUUIDs);
}

void
System::loadLibrary(const u32string& s)
{
  //m_translator->loadLibrary(s);
}

Constant
System::addEquation(const Parser::Equation& eqn)
{
  return addDeclInternal(eqn, m_equations, m_equationUUIDs);
}

Constant 
System::addDimension(const u32string& dimension)
{
  //add equation DIM | [name : "dimension"] = true
  //add equation dimension = Tree::DimensionExpr(U"dimension")
  return addVariableDeclParsed(Parser::Equation(
    dimension,
    Tree::Expr(),
    Tree::Expr(), 
    Tree::DimensionExpr(dimension)
  ));
}

Constant
System::addUnaryOperator(const Tree::UnaryOperator& op)
{
  u32string typeName;

  if (op.type == Tree::UNARY_PREFIX)
  {
    typeName = U"OpPrefix";
  }
  else
  {
    typeName = U"OpPostfix";
  }

  return addOpDeclInternal(op.symbol, Parser::OpDecl{op.symbol,
    Tree::LambdaAppExpr
    (
      Tree::LambdaAppExpr
      (
        Tree::IdentExpr(typeName),
        op.op
      ),
      op.call_by_name
    )
  });

}

Constant
System::addBinaryOperator(const Tree::BinaryOperator& op)
{
  u32string assocType;

  switch(op.assoc)
  {
    case Tree::ASSOC_NON:
    assocType = U"AssocNon";
    break;

    case Tree::ASSOC_LEFT:
    assocType = U"AssocLeft";
    break;

    case Tree::ASSOC_RIGHT:
    assocType = U"AssocRight";
    break;

    default:
    return Types::Special::create(SP_CONST);
    break;
  }

  return addOpDeclInternal(op.symbol, Parser::OpDecl{op.symbol,
    Tree::LambdaAppExpr
    (
      Tree::LambdaAppExpr
      (
        Tree::LambdaAppExpr
        (
          Tree::LambdaAppExpr
          (
            Tree::IdentExpr(U"OpInfix"),
            op.op
          ),
          op.cbn
        ),
        Tree::IdentExpr(assocType)
      ),
      op.precedence
    )
  });
}

uuid
System::addDeclInternal
(
  const u32string& name, 
  const GuardWS& guard, WS* e,
  DefinitionMap& declarations,
  UUIDDefinition& uuids
)
{
  auto i = declarations.find(name);
  VariableWS* var = nullptr;

  if (i == declarations.end())
  {
    var = new VariableWS(name, *this);
    i = declarations.insert(std::make_pair(name, var)).first;
  }
  else
  {
    var = i->second;
  }

  uuid u = var->addEquation(name, guard, e, m_time);

  uuids.insert(std::make_pair(u, i));

  return u;
}

Constant
System::addDeclInternal
(
  const Parser::Equation& eqn, 
  DefinitionMap& declarations,
  UUIDDefinition& uuids
)
{
  //TreeToWSTree tows(this);

  //simplify, turn into workshops
  //Tree::Expr guard   = toWSTreePlusExtras(std::get<1>(eqn), tows);
  //Tree::Expr boolean = toWSTreePlusExtras(std::get<2>(eqn), tows);
  //Tree::Expr expr    = toWSTreePlusExtras(std::get<3>(eqn), tows);

  //simplify, turn into workshops
  Tree::Expr guard   = fixupTreeAndAdd(std::get<1>(eqn));
  Tree::Expr boolean = fixupTreeAndAdd(std::get<2>(eqn));
  Tree::Expr expr    = fixupTreeAndAdd(std::get<3>(eqn));

  #if 0
  if (m_debug)
  {
    //print everything
    std::cout << "The rewritten expr" << std::endl; 
    std::cout << 
      Printer::printEquation
      (std::make_tuple(std::get<0>(eqn), guard, boolean, expr)) 
      << std::endl;

    for (const auto& e : tows.newVars())
    {
      std::cout << Printer::printEquation(e) << std::endl;
    }
  }
  #endif

  WorkshopBuilder compile(this);

  uuid u = addDeclInternal(
    std::get<0>(eqn),
    GuardWS(compile.build_workshops(guard),
      compile.build_workshops(boolean)),
    compile.build_workshops(expr),
    declarations,
    uuids
  );

  //add all the new equations
  #if 0
  for (const auto& e : tows.newVars())
  {
    addDeclInternal(
      std::get<0>(e),
      GuardWS(compile.build_workshops(std::get<1>(e)),
        compile.build_workshops(std::get<2>(e))),
      compile.build_workshops(std::get<3>(e)),
      declarations,
      uuids
    );
  }
  #endif

  return Types::UUID::create(u);
}

Constant
System::addAssignment(const Parser::Equation& eqn)
{
  //return addDeclInternal(eqn, m_assignments, m_assignmentUUIDs);
  uuid u = generate_uuid();

  //find the assignment
  auto assign = m_assignments.find(std::get<0>(eqn));

  if (assign == m_assignments.end())
  {
    assign = m_assignments.insert(
      {std::get<0>(eqn), std::make_shared<Assignment>(std::get<0>(eqn))}
    ).first;
  }

  //simplify, turn into workshops
  Tree::Expr guard   = fixupTreeAndAdd(std::get<1>(eqn));
  Tree::Expr boolean = fixupTreeAndAdd(std::get<2>(eqn));
  Tree::Expr expr    = fixupTreeAndAdd(std::get<3>(eqn));

  WorkshopBuilder compile(this);

  auto guardws = std::shared_ptr<WS>(compile.build_workshops(guard));
  auto booleanws = std::shared_ptr<WS>(compile.build_workshops(boolean));
  auto exprws = std::shared_ptr<WS>(compile.build_workshops(expr));

  assign->second->addDefinition(guardws, booleanws, exprws);

  return Types::UUID::create(u);
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

//parses an expression, returns a tree of the expression as parsed by
//the current definitions of the system
bool
System::parseExpression(Parser::LexerIterator& begin, 
  const Parser::LexerIterator& end,
  Tree::Expr& expr)
{
  return m_parser->parse_expr(begin, end, expr);
}

bool
System::parseExpression(Parser::StreamPosIterator& iter, 
  const Parser::StreamPosIterator& end,
  Tree::Expr& expr)
{
  Parser::LexerIterator lexit(iter, end, m_defaultk, lookupIdentifiers());
  auto lexend = lexit.makeEnd();
  
  return parseExpression(lexit, lexend, expr);
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

  //set the rho dimension to nil
  auto nil = lookupIdentifiers().lookup(U"Nil");
  
  if (nil != nullptr)
  {
    auto nilvalue = (*nil)(m_defaultk);

    if (nilvalue.index() == TYPE_INDEX_TUPLE)
    {
      m_defaultk.perturb(DIM_RHO, nilvalue);
    }
  }

  for (const auto& v : m_envvars)
  {
    m_defaultk.perturb(v.first, v.second);
  }
}

#if 0
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
#endif

Constant
System::evalExpr(const Tree::Expr& e)
{
  return compile_and_evaluate(e, *this);
}

u32string
System::printDimension(dimension_index dim) const
{
  const u32string* name = m_dimTranslator.reverse_lookup_named(dim);

  if (name != nullptr)
  {
    return *name;
  }

  const Constant* val = m_dimTranslator.reverse_lookup_constant(dim);

  if (val != nullptr)
  {
    auto printer = m_equations.find(U"CANONICAL_PRINT");

    Context k = m_defaultk;

    ContextPerturber p(k, {{DIM_ARG0, *val}});

    Constant string = printer->second->operator()(k);

    if (string.index() != TYPE_INDEX_USTRING)
    {
      return U"error printing dimension";
    }
    else
    {
      return get_constant_pointer<u32string>(string);
    }
  }

  std::ostringstream os;
  os << dim;

  return utf8_to_utf32(os.str());
}

void
System::addEnvVars()
{
  char** envvar = environ;

  while (*envvar != nullptr)
  {
    std::string var = *envvar;
    size_t equals = var.find('=');

    //std::cerr << "variable: " << var << std::endl;
    //std::cerr << "equals is at " << equals << std::endl;
    std::string varname = var.substr(0, equals);
    std::string varvalue = var.substr(equals + 1, std::string::npos);
    //std::cerr << "read in: " << varname << ", " << varvalue << std::endl;

    u32string u32name(varname.begin(), varname.end());

    addEnvVar(u32name, 
      Types::String::create(u32string(varvalue.begin(), varvalue.end())));

    ++envvar;
  }
}

void
System::addEnvVar(const u32string& name, const Constant& value)
{
  m_envvars.insert({getDimensionIndex(name), value});

  addDimension(name);
}

Constant
System::addHostDimension(const u32string& name, dimension_index index)
{
  if (m_dimTranslator.assignIndex(name, index))
  {
    return addVariableDeclParsed(Parser::Equation
      (
        name,
        Tree::Expr(),
        Tree::Expr(),
        Tree::DimensionExpr(index)
      ));
  }
  else
  {
    return Types::Special::create(SP_CONST);
  }
}

Constant
System::addHostFunction
  (const u32string& name, BaseFunctionType* address, int arity)
{
  //m_baseCounter keeps the number of times this function has been
  //defined, and m_basefuns is the actual definitions where the name has
  //been appended with a version number
  std::shared_ptr<BaseFunctionType> theclone(address->clone());

  auto iter = m_baseCounter.find(name);

  size_t count = 0;

  if (iter == m_baseCounter.end())
  {
    m_baseCounter.insert(std::make_pair(name, 0));
  }
  else
  {
    count = iter->second;
    ++iter->second;
  }

  std::ostringstream os;
  os << name << "_" << count;

  auto newname = os.str();

  auto newname32 = u32string(newname.begin(), newname.end());

  m_basefuns.insert(std::make_pair(newname32, theclone));

  //then add to m_identifiers
  //name = VariableWS
  //and add Tree::basefun(name_counter) to its definition
  //
  //for several additions of the base function through time we will have
  //name = basefun(name_0)
  //name = basefun(name_1)
  //name = basefun(name_2)
  //...

  return addVariableDeclParsed(
    Parser::Equation
    (
      name,
      Tree::nil(),
      Tree::nil(),
      Tree::BaseAbstractionExpr(newname32)
    )
  );

  //std::unique_ptr<BangAbstractionWS> 
  //  op(new BangAbstractionWS(theclone.get()));

  //add equation fn.op_name = bang abstraction workshop with fn.fn
  //uuid u = addEquation(name, op.get());
  //Constant c = Types::UUID::create(u);

  //m_functionRegistry.insert({name, std::make_tuple(theclone.get(), u)});

  //op.release();
  //theclone.release();

  //return c;
}

void
System::addHostTypeIndex(type_index index, const u32string& name)
{
  m_typeRegistry.assignIndex(name, index);
}

bool
System::deleteEntity(const uuid& u)
{
  //only look for equations at the moment
  auto iter = m_equationUUIDs.find(u);

  if (iter == m_equationUUIDs.end())
  {
    return false;
  }

  return false;
  //return iter->second->second->delexpr(u, m_time);
}

u32string
System::printConstant(const Constant& c)
{
  auto iter = m_equations.find(U"canonical_print");

  if (iter == m_equations.end())
  {
    return utf8_to_utf32(_("no print function defined"));
  }

  Constant func = (*iter->second)(m_defaultk);

  Constant result = applyFunction(m_defaultk, func, c); 

  if (result.index() != TYPE_INDEX_USTRING)
  {
    return utf8_to_utf32(_("canonical_print didn't return a string"));
  }

  return Types::String::get(result);
}

void
System::cacheVar(const u32string& name)
{
  //first find the variable in the equations
  auto thevar = m_equations.find(name);

  if (thevar == m_equations.end())
  {
    return;
  }

  //then make sure we haven't already cached it
  auto cachevar = m_cachedVars.find(name);

  if (cachevar == m_cachedVars.end())
  {
    std::unique_ptr<Workshops::CacheWS> cachews
    {
      new Workshops::CacheWS(thevar->second, name, *this)
    };
    m_cachedVars.insert({name, cachews.get()});
    cachews.release();
  }
}

void
System::cacheIfVar(const uuid& id)
{
  auto uiter = m_equationUUIDs.find(id);

  if (uiter != m_equationUUIDs.end())
  {
    cacheVar(uiter->second->first);
  }
}

WS*
System::IdentifierLookup::lookup(const u32string& name) const
{
  //first look for a cached version of this
  auto cache = m_cached->find(name);

  if (cache != m_cached->end())
  {
    return cache->second;
  }

  auto r = m_identifiers->find(name);
  if (r != m_identifiers->end())
  {
    return r->second.get();
  }
  else
  {
    return nullptr;
  }
}

void
System::disableCache()
{
  m_cacheEnabled = false;
}

void 
System::enableCache()
{
  m_cacheEnabled = true;
}

bool
System::cacheEnabled() const
{
  return m_cacheEnabled;
}

void
System::addTransformedEquations
(
  const std::vector<Parser::Equation>& newVars
)
{
  //add all the new equations
  //std::cerr << "adding extra equations" << std::endl;
  for (const auto& e : newVars)
  {
    addVariableDeclParsed(e);
  }
}

Tree::Expr
System::fixupTreeAndAdd(const Tree::Expr& e)
{
  auto result = fixupTree(*this, e);
  addTransformedEquations(result.second.equations);

  return result.first;
}

BaseFunctionType*
System::lookupBaseFunction(const u32string& name)
{
  auto iter = m_basefuns.find(name);
  if (iter == m_basefuns.end())
  {
    return nullptr;
  }
  else
  {
    return iter->second.get();
  }
}

Constant
System::addDeclaration(const Parser::RawInput& input)
{
  //create a U32Iterator for the input
  Parser::U32Iterator inputBegin(
    Parser::makeUTF32Iterator(input.text.begin())
  );
  Parser::U32Iterator inputEnd(
    Parser::makeUTF32Iterator(input.text.end())
  );

  //create a stream pos iterator for the U32Iterator
  Parser::StreamPosIterator posbegin(inputBegin, input.source,
    input.line, input.character);
  Parser::StreamPosIterator posend(inputEnd);

  //create a lexer iterator
  Parser::LexerIterator lexit(posbegin, posend, m_defaultk, 
    lookupIdentifiers());
  Parser::LexerIterator lexend = lexit.makeEnd();

  //get the first token
  auto start = *lexit;

  if (start.getType() != Parser::TOKEN_DECLID)
  {
    return Types::Special::create(SP_ERROR);
  }
  
  u32string token = get<u32string>(start.getValue());

  if (token == U"var")
  {
    return addVariableDeclRaw(input, lexit);
  }
  else if (token == U"dim")
  {
    return addDimDeclRaw(input, lexit);
  }
  else if (token == U"fun")
  {
    return addFunDeclRaw(input, lexit);
  }
  else if (token == U"op")
  {
    return addOpDeclRaw(input, lexit);
  }
  else if (token == U"data")
  {
    return addDataDeclRaw(input, lexit);
  }
  else if (token == U"constructor")
  {
    return addConstructorRaw(input, lexit);
  }

  return Constant();
}

Constant
System::addVariableDeclRaw
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  ++iter;
  if (iter->getType() != Parser::TOKEN_ID)
  {
    return Types::Special::create(SP_ERROR);
  }

  u32string name = get<u32string>(iter->getValue());

  return addVariableDeclInternal(name, input);
}

Constant
System::addVariableDeclParsed
(
  Parser::Equation decl
)
{
  return addVariableDeclInternal(std::get<0>(decl), Parser::Variable(decl));
}

Constant
System::addFunDeclRaw
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  ++iter;
  if (iter->getType() != Parser::TOKEN_ID)
  {
    return Types::Special::create(SP_ERROR);
  }

  u32string name = get<u32string>(iter->getValue());

  return addFunDeclInternal(name, input);
}

Constant
System::addFunDeclParsed
(
  Parser::FnDecl decl
)
{
  return addFunDeclInternal(decl.name, decl);
}

Constant
System::addOpDeclRaw
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  iter.interpret(false);
  ++iter;

  if (iter->getType() != Parser::TOKEN_OPERATOR)
  {
    return Types::Special::create(SP_ERROR);
  }

  u32string text = get<u32string>(iter->getValue());

  return addOpDeclInternal(text, input);
}

Constant
System::addDataDeclRaw
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  ++iter;
  if (iter->getType() != Parser::TOKEN_ID)
  {
    return Types::Special::create(SP_ERROR);
  }

  u32string name = get<u32string>(iter->getValue());

  return addVariableDeclParsed(Parser::Equation
  (
    name,
    Tree::Expr(),
    Tree::Expr(),
    Tree::TupleExpr(Tree::TupleExpr::TuplePairs({
      {Tree::DimensionExpr{DIM_TYPE}, name}
    }))
  ));
}

Constant
System::addConstructorRaw
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  ++iter;
  if (iter->getType() != Parser::TOKEN_ID)
  {
    return Types::Special::create(SP_ERROR);
  }

  u32string name = get<u32string>(iter->getValue());

  return addConstructorInternal(name, input);
}

Constant
System::addDimDeclRaw
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
   ++iter;
  if (iter->getType() != Parser::TOKEN_ID)
  {
    return Types::Special::create(SP_ERROR);
  }

  u32string name = get<u32string>(iter->getValue());

  return addVariableDeclParsed(Parser::Equation
  (
    name, 
    Tree::Expr(), 
    Tree::Expr(),
    Tree::DimensionExpr(nextHiddenDim())
  ));
}

} //namespace TransLucid
