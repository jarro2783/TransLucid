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

/**
 * @file system.hpp
 * The System.
 */

#ifndef SYSTEM_HPP_INCLUDED
#define SYSTEM_HPP_INCLUDED

#include <tl/ast_fwd.hpp>
#include <tl/chi.hpp>
#include <tl/datadef.hpp>
#include <tl/dimtranslator.hpp>
#include <tl/types.hpp>
#include <tl/equation.hpp>
#include <tl/function.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/opdef.hpp>
#include <tl/parser_api.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/registries.hpp>
#include <tl/semantics.hpp>
#include <tl/system_object.hpp>
#include <tl/trie.hpp>

#include <unordered_set>
#include <unordered_map>

namespace TransLucid
{

  namespace detail
  {
    class InputHDWS;
    class LineAdder;
  }

  namespace Parser
  {
    class Parser;
    struct FnDecl;

    class LexerIterator;
  }

  class StreamPosIterator;

  constexpr int MAX_FUNCTION_PARAMETERS = 10;

  class Translator;
  class TreeToWSTree;
  class SemanticTransform;

  class BaseFunctionType;

  class Assignment;

  struct ExtraTreeInformation;

  struct GettextInit
  {
    GettextInit();
  };

  namespace Workshops
  {
    class CacheWS;
  }

  /**
   * @brief System base class.
   *
   * Holds all the data necessary for an system.
   **/
  class System : public TypeRegistry, public DimensionRegistry
  {
    public:
    typedef std::unordered_map<u32string, VariableWS*> DefinitionMap;
    typedef std::unordered_map<u32string, std::shared_ptr<WS>> IdentifierMap;
    typedef std::unordered_map<uuid, std::shared_ptr<SystemObject>> ObjectMap;
    typedef std::unordered_map<u32string, std::shared_ptr<VariableWS>> 
      VariableMap;
    typedef std::unordered_map<u32string, std::shared_ptr<FunctionWS>> 
      FunctionMap;
    typedef std::unordered_map<u32string, std::shared_ptr<OpDefWS>> 
      OperatorMap;
    typedef std::unordered_map<u32string, std::shared_ptr<ConsDefWS>> 
      ConstructorMap;
    typedef std::unordered_map<u32string, std::shared_ptr<Assignment>>
      AssignmentMap;

    System(bool cached = false);
    ~System();

    //don't want to copy
    System(const System&) = delete;

    //the registry interface
    type_index
    getTypeIndex(const u32string& name);

    dimension_index
    getDimensionIndex(const u32string& name);

    dimension_index
    getDimensionIndex(const Constant& c);

    u32string
    printDimension(dimension_index dim) const;

    //generate a new dimension index
    dimension_index
    nextDimensionIndex()
    {
      return m_dimTranslator.unique();
    }

    dimension_index
    nextHiddenDim()
    {
      return m_dimTranslator.unique();
      //return m_hiddenDim--;
    }

    dimension_index
    getChiDim(const ChiDim& dim)
    {
      return m_chiMap.lookup(dim);
    }

    /**
     * Get the time.
     * @return The current time of the system.
     */
    size_t 
    theTime()
    {
      return m_time;
    }

    /**
     * Commit the current changes. This has the effect of incrementing to the
     * next time.
     */
    void
    commit()
    {
      ++m_time;
    }

    //string input?
    #if 0
    uuid
    addEquation(const u32string& name, const GuardWS& guard, WS* e);

    uuid
    addEquation(const u32string& name, WS* e)
    {
      return addEquation(name, GuardWS(), e);
    }
    #endif

    Constant
    addHostDimension(const u32string& name, dimension_index index);

    Constant
    addHostFunction
      (const u32string& name, BaseFunctionType* address, int arity);

    Constant
    addEquation(const Parser::Equation& eqn);

    Constant
    addAssignment(const Parser::Equation& eqn);

    Constant 
    addDimension(const u32string& dimension);

    Constant
    addBinaryOperator(const Tree::BinaryOperator& op);

    Constant
    addUnaryOperator(const Tree::UnaryOperator& op);

    Constant
    addOutputHyperdaton
    (
      const u32string& name,
      OutputHD* hd
    );

    //Constant
    //addInputHyperdaton
    //(
    //  const u32string& name,
    //  InputHD* hd
    //);

    //parses an expression, returns a tree of the expression as parsed by
    //the current definitions of the system
    bool
    parseExpression(Parser::StreamPosIterator& begin, 
      const Parser::StreamPosIterator& end,
      Tree::Expr& expr);

    //parses an expression, returns a tree of the expression as parsed by
    //the current definitions of the system
    bool
    parseExpression(Parser::LexerIterator& begin, 
      const Parser::LexerIterator& end,
      Tree::Expr& expr);

    void
    addTransformedEquations
    (
      const ExtraTreeInformation& newVars
    );

    void
    addParsedDecl(const Parser::Line& decl, ScopePtr scope);

    void
    loadLibrary(const u32string& s);

    void
    go();

    void
    addEnvVars();

    void
    addEnvVar(const u32string& name, const Constant& value);

    void
    addHostTypeIndex(type_index index, const u32string& name);

    bool
    deleteEntity(const uuid& u);

    u32string
    printConstant(const Constant& c);

    void
    cacheVar(const u32string& name);

    void
    cacheIfVar(const uuid& id);

    void
    disableCache();

    void 
    enableCache();

    bool
    cacheEnabled() const;

    Tree::Expr
    fixupTreeAndAdd(const Tree::Expr& e, ScopePtr scope = ScopePtr());

    BaseFunctionType*
    lookupBaseFunction(const u32string& name);

    OutputHD*
    getOutputHD(const u32string& name)
    {
      auto iter = m_outputHDs.find(name);
      return iter == m_outputHDs.end() ? nullptr : iter->second;
    }

    int
    nextWhere()
    {
      return m_whereCounter++;
    }

    private:

    typedef std::unordered_map<u32string, size_t> BaseFunctionCounter;
    typedef std::unordered_map<
      u32string, 
      std::shared_ptr<BaseFunctionType>
    > BaseFunctionDefinitions;

    //definitions of Equations
    typedef std::unordered_map<uuid, DefinitionMap::iterator> UUIDDefinition;

    //output hyperdatons
    typedef std::unordered_map<u32string, OutputHD*> OutputHDMap;
    //input hyperdatons
    typedef std::unordered_map<u32string, InputHD*> InputHDMap;

    //uuid to string
    typedef std::unordered_map<uuid, u32string>
      UUIDStringMap;

    // OPTYPE, ATL_SYMBOL
    typedef std::tuple<uuid, uuid> UnaryUUIDs;

    // OPTYPE, ATL_SYMBOL, ASSOC, PREC
    typedef std::tuple<uuid, uuid, uuid, uuid> BinaryUUIDs;

    //set of uuids
    typedef std::unordered_set<uuid> UUIDHashSet;

    //uuids of set of uuids of unary operator equations
    typedef std::unordered_map<uuid, UnaryUUIDs> 
      UnaryUUIDSet;

    //uuids of set of uuids of binary operator equations
    typedef std::unordered_map<uuid, BinaryUUIDs>
      BinaryUUIDSet;

    //initialises the type indexes
    void
    init_types();

    void 
    init_dimensions(const std::initializer_list<u32string>& args);

    template <typename T>
    WS*
    buildConstantWS(size_t index);

    // -- internal add functions --

    template <typename T>
    Tuple
    addHDDecl
    (
      const u32string& name,
      const Tree::Expr& guard,
      T& decls
    );

    void
    setDefaultContext();

    template <typename... Renames>
    Tree::Expr
    toWSTreePlusExtras(const Tree::Expr& e, TreeToWSTree& tows,
      Renames&&... renames);

    bool m_cached;
    bool m_cacheEnabled;

    ObjectMap m_objects;
    IdentifierMap m_identifiers;

    VariableMap m_variables;
    FunctionMap m_functions;
    std::shared_ptr<OpDefWS> m_operators;
    ConstructorMap m_constructors;
    AssignmentMap m_assignments;

    //base functions
    BaseFunctionCounter m_baseCounter;
    BaseFunctionDefinitions m_basefuns;

    //TODO deprecating
    DefinitionMap m_equations;

    UUIDDefinition m_equationUUIDs;
    UUIDDefinition m_assignmentUUIDs;

    //the variables that we want to cache
    std::unordered_map<u32string, Workshops::CacheWS*> m_cachedVars;

    //maps of string to hds and the hds uuids
    OutputHDMap m_outputHDs;
    UUIDStringMap m_outputUUIDs;

    InputHDMap m_inputHDs;

    //input and output hd declarations, for now just have the valid range
    std::unordered_map<u32string, Tuple> m_outputHDDecls;
    std::unordered_map<u32string, Tuple> m_inputHDDecls;

    //---- the sets of all the uuids of objects ----

    //the uuids of the dimensions
    UUIDHashSet m_dimension_uuids;

    //the uuids of the unary operators
    UnaryUUIDSet m_unop_uuids;

    //the uuids of the binary operators
    BinaryUUIDSet m_binop_uuids;

    //registries
    type_index m_nextTypeIndex;
    ObjectRegistry<u32string, decltype(m_nextTypeIndex), Decrement<type_index>> 
      m_typeRegistry;
    DimensionTranslator m_dimTranslator;

    std::unordered_map<u32string, std::tuple<BaseFunctionType*, uuid>>
      m_functionRegistry;

    Context m_defaultk;
    size_t m_time;
    //Translator *m_translator;
    std::vector<dimension_index> m_Lin;
    std::vector<dimension_index> m_fnLists;
    std::map<dimension_index, Constant> m_envvars;

    size_t m_uniqueVarIndex;
    size_t m_uniqueDimIndex;
    dimension_index m_hiddenDim;
    int m_whereCounter;

    bool m_debug;
    bool m_verbose;

    Parser::Parser* m_parser;

    ChiMap m_chiMap;

    static GettextInit m_gettext;

    friend class detail::InputHDWS;
    friend class detail::LineAdder;

    template <typename Input>
    Constant
    addVariableDeclInternal
    (
      const u32string& name,
      Input&& decl,
      ScopePtr = ScopePtr()
    );

    template <typename Input>
    Constant
    addFunDeclInternal
    (
      const u32string& name,
      Input&& decl,
      ScopePtr scope = ScopePtr()
    );

    template <typename Input>
    Constant
    addOpDeclInternal
    (
      const u32string& name,
      Input&& decl
    );

    template <typename Input>
    Constant
    addConstructorInternal
    (
      const u32string& name,
      Input&& decl
    );

    public:

    Constant
    addDeclaration(const Parser::RawInput& input);

    Constant
    addOpDeclRaw
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    addFunDeclParsed
    (
      Parser::FnDecl decl
    );

    Constant
    addFunDeclRaw
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    addVariableDeclRaw
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    addVariableDeclParsed
    (
      Parser::Equation decl
    );

    Constant
    addDimDeclRaw
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    addDataDeclRaw
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    addConstructorRaw
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    delDecl
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    Constant
    replDecl
    (
      const Parser::RawInput& input, 
      Parser::LexerIterator& iter
    );

    size_t
    nextVarIndex()
    {
      return m_uniqueVarIndex++;
    }

    size_t
    nextDimIndex()
    {
      return m_uniqueDimIndex++;
    }

    Context&
    getDefaultContext()
    {
      return m_defaultk;
    }

    struct IdentifierLookup
    {
      IdentifierLookup
      (
        IdentifierMap& identifiers,
        decltype(m_cachedVars)& cached
      )
      : m_identifiers(&identifiers),
        m_cached(&cached)
      {
      }

      IdentifierLookup()
      : m_identifiers(nullptr)
      {}
      
      operator bool() const
      {
        return m_identifiers != nullptr;
      }

      WS*
      lookup(const u32string& name) const;

      private:
      IdentifierMap* m_identifiers;
      decltype(m_cachedVars)* m_cached;
    };

    IdentifierLookup lookupIdentifiers()
    {
      return IdentifierLookup(m_identifiers, m_cachedVars);
    }

    //template <size_t N>
    //auto
    //lookupFunction(const u32string& name)
    //  -> decltype(m_functions.lookupFunction<N>(name))
    //{
    //  return m_functions.lookupFunction<N>(name);
    //}

  };

  Constant hash(const Constant& dimension, const Tuple& context);
  Constant hash(size_t dimension, const Tuple& context);
}

#endif // SYSTEM_HPP_INCLUDED
