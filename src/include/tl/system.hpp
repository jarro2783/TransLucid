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

#include <unordered_set>
#include <unordered_map>

#include <tl/ast_fwd.hpp>
#include <tl/dimtranslator.hpp>
#include <tl/types.hpp>
#include <tl/equation.hpp>
#include <tl/free_variables.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/parser_api.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/registries.hpp>
#include <tl/system_object.hpp>

namespace TransLucid
{

  namespace detail
  {
    class InputHDWS;
  }

  namespace Parser
  {
    class Parser;
    class FnDecl;

    class LexerIterator;
  }

  class StreamPosIterator;

  constexpr int MAX_FUNCTION_PARAMETERS = 10;

  class Translator;
  class TreeToWSTree;
  class SemanticTransform;

  class BaseFunctionType;

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
    typedef std::unordered_map<uuid, SystemObject*> ObjectMap;

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
    uuid
    addEquation(const u32string& name, const GuardWS& guard, WS* e);

    uuid
    addEquation(const u32string& name, WS* e)
    {
      return addEquation(name, GuardWS(), e);
    }

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
    addFunction(const Parser::FnDecl& fn);

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

    Constant
    addInputHyperdaton
    (
      const u32string& name,
      InputHD* hd
    );

    void
    addOutputDeclaration
    (
      const u32string& name,
      const Tree::Expr& guard
    );

    void
    addInputDeclaration
    (
      const u32string& name,
      const Tree::Expr& guard
    );

    Constant
    parseLine
    (
      Parser::StreamPosIterator& begin, 
      const Parser::StreamPosIterator& end,
      int verbose = 1, 
      bool debug = false
    );

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
      const std::vector<Parser::Equation>& newVars
    );

    void
    loadLibrary(const u32string& s);

    void
    go();

    Constant
    evalExpr(const Tree::Expr& e);

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
    fixupTreeAndAdd(const Tree::Expr& e);

    BaseFunctionType*
    lookupBaseFunction(const u32string& name);

    private:
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

    //initialises the default equations
    void
    init_equations();

    void 
    init_dimensions(const std::initializer_list<u32string>& args);

    template <typename T>
    WS*
    buildConstantWS(size_t index);

    // -- internal add functions --

    template <typename T>
    Constant
    addSymbolInfo
    (
      const u32string& eqn, 
      const u32string& s, 
      const T& value
    );

    template <typename T>
    Tuple
    addHDDecl
    (
      const u32string& name,
      const Tree::Expr& guard,
      T& decls
    );

    Constant 
    addOpType(const u32string& symbol, const u32string& type);

    Constant
    addATLSymbol(const u32string& symbol, const u32string& op);

    Constant
    addAssoc(const u32string& symbol, const u32string assoc);

    Constant
    addPrecedence(const u32string& symbol, const mpz_class& precedence);

    uuid
    addDeclInternal
    (
      const u32string& name, 
      const GuardWS& guard, WS* e,
      DefinitionMap& declarations,
      UUIDDefinition& uuids
    );

    Constant
    addDeclInternal(const Parser::Equation& eqn, DefinitionMap& declarations,
      UUIDDefinition& uuids
    );

    void
    setDefaultContext();

    template <typename... Renames>
    Tree::Expr
    toWSTreePlusExtras(const Tree::Expr& e, TreeToWSTree& tows,
      Renames&&... renames);

    void
    addDefaultDimensions
    (
      const std::vector<dimension_index>& zeros,
      const std::vector<dimension_index>& nils
    );

    void
    addDefaultDimensions(const SemanticTransform& t);

    bool m_cached;
    bool m_cacheEnabled;

    ObjectMap m_objects;
    DefinitionMap m_equations;
    UUIDDefinition m_equationUUIDs;
    DefinitionMap m_assignments;
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

    typedef
      std::tuple
      <
        ConditionalBestfitWS*, 
        std::map<u32string, u32string>,
        std::map<u32string, dimension_index>,
        std::vector<Parser::FnDecl>,
        WS*,
        FreeVariableHelper
      >
    FnInfo;

    //functions
    //map from names to a tuple of
    //(bestfitter, renamed arguments, args -> dim, definitions,
    // top abstraction workshop, free var replacer)
    std::unordered_map
    <
      u32string, 
      FnInfo
    > m_fndecls;

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

    bool m_debug;
    bool m_verbose;

    Parser::Parser* m_parser;

    static GettextInit m_gettext;

    friend class detail::InputHDWS;

    Tree::Expr
    funWSTree
    (
      FnInfo& info, 
      const Parser::FnDecl& decl,
      const Tree::Expr& expr, 
      WS* abstraction
    );

    public:

    Constant
    addDeclaration(const Parser::RawInput& input);

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
        DefinitionMap& identifiers,
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
      DefinitionMap* m_identifiers;
      decltype(m_cachedVars)* m_cached;
    };

    IdentifierLookup lookupIdentifiers()
    {
      return IdentifierLookup(m_equations, m_cachedVars);
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
