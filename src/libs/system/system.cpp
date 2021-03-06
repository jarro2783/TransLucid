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

extern char** environ;

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
    repl(uuid id, int time, const Parser::RawInput& line)
    {
      return m_object->repl(id, time, line);
    }

    bool
    repl(uuid id, int time, const Parser::Line& line)
    {
      return m_object->repl(id, time, line);
    }

    void
    cache()
    {
      return m_object->cache();
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

  class LineAdder
  {
    public:

    typedef void result_type;

    LineAdder(System& system)
    : m_system(system)
    {
    }

    void
    add(const Parser::Line& line, ScopePtr scope)
    {
      apply_visitor(*this, line, scope);
    }

    template <typename T>
    void
    operator()(const T&, ScopePtr scope)
    {
    }

    void
    operator()(const Parser::FnDecl& fun, ScopePtr scope)
    {
      m_system.addFunDeclInternal(fun.name, fun, scope);
    }

    void
    operator()(const Parser::Variable& var, ScopePtr scope)
    {
      m_system.addVariableDeclInternal(std::get<0>(var.eqn), var, scope);
    }

    private:
    System& m_system;
  };
}

//private template  functions go at the top, but after the local
//definitions

template <typename Input>
Constant
System::addVariableDeclInternal
(
  const u32string& name,
  Input&& decl,
  ScopePtr scope
)
{
  uuid u = generate_uuid();

  auto equationIter = m_variables.find(name);

  std::shared_ptr<VariableWS> var;
  if (equationIter == m_variables.end())
  {
    var = std::make_shared<VariableWS>(name, *this);

    if (cached())
    {
      var->cache();
    }

    auto variter = m_variables.insert({name, var});

    //create a new VariableObject to manage the uuid
    m_objects.insert({u, make_generic_object(variter.first->second)});
  }
  else
  {
    var = equationIter->second;
  }
  var->addEquation(u, std::forward<Input>(decl), m_time, scope);

  m_identifiers.insert({name, var});

  return Types::UUID::create(u);
}

template <typename Input>
Constant
System::addFunDeclInternal
(
  const u32string& name,
  Input&& decl,
  ScopePtr scope
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

  fun->addEquation(u, decl, m_time, scope);

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

void 
System::init_dimensions(const std::initializer_list<u32string>& args)
{
  for (auto s : args)
  {
    addDimension(s);
  }
}

//TODO this type registry stuff should go somewhere that is easier to find
System::System(bool cached, bool simplify)
: 
  m_cacheEnabled(cached),
  m_simplified(simplify),
  m_nextTypeIndex(-1),
  m_typeRegistry(m_nextTypeIndex,
  std::vector<std::pair<u32string, type_index>>{
   type_name_pair(TYPE_INDEX_ERROR),
   type_name_pair(TYPE_INDEX_USTRING),
   type_name_pair(TYPE_INDEX_INTMP),
   type_name_pair(TYPE_INDEX_FLOATMP),
   type_name_pair(TYPE_INDEX_BOOL),
   type_name_pair(TYPE_INDEX_SPECIAL),
   type_name_pair(TYPE_INDEX_UCHAR),
   type_name_pair(TYPE_INDEX_DIMENSION),
   type_name_pair(TYPE_INDEX_INFINITY),
   type_name_pair(TYPE_INDEX_TUPLE),
   type_name_pair(TYPE_INDEX_TYPE),
   type_name_pair(TYPE_INDEX_RANGE),
   type_name_pair(TYPE_INDEX_VALUE_FUNCTION),
   type_name_pair(TYPE_INDEX_NAME_FUNCTION),
   type_name_pair(TYPE_INDEX_UUID),
   type_name_pair(TYPE_INDEX_DEMAND),
   type_name_pair(TYPE_INDEX_CALC),
   type_name_pair(TYPE_INDEX_BASE_FUNCTION),
   type_name_pair(TYPE_INDEX_UNION),
   type_name_pair(TYPE_INDEX_INTENSION)
  }
  )
, m_time(0)
, m_uniqueVarIndex(0)
, m_uniqueDimIndex(0)
, m_hiddenDim(-1)
, m_whereCounter(0)
, m_debug(false)
, m_chiMap(*this)
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

void
System::addParsedDecl(const Parser::Line& decl, ScopePtr scope)
{
  detail::LineAdder adder(*this);
  adder.add(decl, scope);
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

Constant
System::addAssignment(const Parser::Equation& eqn)
{
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

  if (cached())
  {
    auto guardws = std::make_shared<Workshops::CacheWS>
      (
        compile.build_workshops(guard),
        U"assignment_guard",
        *this
      );
    auto booleanws = std::make_shared<Workshops::CacheWS>
      (
        compile.build_workshops(boolean),
        U"assignment_boolean",
        *this
      );
    auto exprws = std::make_shared<Workshops::CacheWS>
      (
        compile.build_workshops(expr),
        U"assignment_expr",
        *this
      );

    assign->second->addDefinition(Assignment::Definition
      {
        guard, boolean, expr,
        guardws, booleanws, exprws
      });
  }
  else
  {
    auto guardws = std::shared_ptr<WS>(compile.build_workshops(guard));
    auto booleanws = std::shared_ptr<WS>(compile.build_workshops(boolean));
    auto exprws = std::shared_ptr<WS>(compile.build_workshops(expr));

    assign->second->addDefinition(Assignment::Definition
      {
        guard, boolean, expr,
        guardws, booleanws, exprws
      });
  }

  return Types::UUID::create(u);
}

type_index
System::getTypeIndex(const u32string& name)
{
  return m_typeRegistry(name);
}

u32string
System::getTypeName(type_index t)
{
  auto n = m_typeRegistry.reverseLookup(t);

  if (n)
  {
    return *n;
  }
  else
  {
    return U"";
  }
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
      Tree::HostOpExpr(newname32)
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
  auto iter = m_identifiers.find(U"canonical_print");

  if (iter == m_identifiers.end())
  {
    std::ostringstream os;
    os << "Constant(" << c.index() << ")";
    return utf8_to_utf32(os.str());
  }

  Constant func = (*iter->second)(m_defaultk);

  Constant result = applyFunction<FUN_VALUE>(m_defaultk, func, c); 

  if (result.index() != TYPE_INDEX_USTRING)
  {
    std::ostringstream os;
    os << _("canonical_print error: ") << "Constant(" 
      << *m_typeRegistry.reverseLookup(c.index()) << ")";
    return utf8_to_utf32(os.str());
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
System::cacheObject(const uuid& id)
{
  auto uiter = m_objects.find(id);

  if (uiter != m_objects.end())
  {
    uiter->second->cache();
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
  const ExtraTreeInformation& newVars
)
{
  //add all the new equations
  //std::cerr << "adding extra equations" << std::endl;
  for (const auto& e : newVars.equations)
  {
    addParsedDecl(e.second, e.first);
  }
}

Tree::Expr
System::fixupTreeAndAdd(const Tree::Expr& e, ScopePtr scope)
{
  auto result = fixupTree(*this, e, scope);
  addTransformedEquations(result.second);

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
  else if (token == U"del")
  {
    return delDecl(input, lexit);
  }
  else if (token == U"repl")
  {
    return replDecl(input, lexit);
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
    Tree::RegionExpr(Tree::RegionExpr::Entries({
      std::make_tuple(
        Tree::DimensionExpr{DIM_TYPE}, 
        Region::Containment::IS, 
        name
      )
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

Constant
System::delDecl
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  ++iter;

  if (iter->getType() != Parser::TOKEN_CONSTANT)
  {
    return Types::Special::create(SP_ERROR);
  }

  auto value = get<std::pair<u32string, u32string>>(iter->getValue());

  if (value.first != U"uuid")
  {
    return Types::Special::create(SP_ERROR);
  }

  uuid id;
  try
  {
    id = parse_uuid(value.second);
  }
  catch (InvalidUUID&)
  {
    return Types::Special::create(SP_ERROR);
  }

  auto object = m_objects.find(id);

  if (object == m_objects.end())
  {
    return Types::Special::create(SP_UNDEF);
  }

  return Types::Boolean::create(object->second->del(id, m_time));
}

Constant
System::replDecl
(
  const Parser::RawInput& input, 
  Parser::LexerIterator& iter
)
{
  ++iter;

  if (iter->getType() != Parser::TOKEN_CONSTANT)
  {
    return Types::Special::create(SP_ERROR);
  }

  auto value = get<std::pair<u32string, u32string>>(iter->getValue());

  if (value.first != U"uuid")
  {
    return Types::Special::create(SP_ERROR);
  }

  uuid id;
  try
  {
    id = parse_uuid(value.second);
  }
  catch (InvalidUUID&)
  {
    return Types::Special::create(SP_ERROR);
  }

  auto object = m_objects.find(id);

  if (object == m_objects.end())
  {
    return Types::Special::create(SP_UNDEF);
  }

  return Types::Boolean::create(object->second->repl(id, m_time, input));
}

Tree::Expr
System::getIdentifierTree(const u32string& x)
{
  //currently we only look in variables and functions

  auto variter = m_variables.find(x);
  if (variter != m_variables.end())
  {
    return variter->second->getEquation(m_defaultk);
  }

  auto funiter = m_functions.find(x);
  if (funiter != m_functions.end())
  {
    return funiter->second->getEquation(m_defaultk);
  }

  return Tree::Expr();
}

} //namespace TransLucid
