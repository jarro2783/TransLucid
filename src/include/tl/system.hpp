/* The System.
   Copyright (C) 2009-2011 Jarryd Beck and John Plaice

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

#ifndef SYSTEM_HPP_INCLUDED
#define SYSTEM_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/dimtranslator.hpp>
#include <tl/equation.hpp>
#include <tl/function_registry.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/parser_api.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/registries.hpp>

#include <unordered_set>
#include <unordered_map>

#include <boost/function.hpp>

/**
 * @file system.hpp
 * The System Hyperdaton header file.
 */

namespace TransLucid
{

  namespace detail
  {
    class InputHDWS;
  }

  constexpr int MAX_FUNCTION_PARAMETERS = 10;

  template <typename T>
  class uuidmap : public std::map<uuid, T> {
  };

  class Translator;

  /**
   * @brief System base class.
   *
   * Holds all the data necessary for an system.
   **/
  class System : public TypeRegistry, public DimensionRegistry
  {
    public:

    System();
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
    parseLine(Parser::U32Iterator& begin, bool verbose = false);

    //parses an expression, returns a tree of the expression as parsed by
    //the current definitions of the system
    std::pair<bool, Tree::Expr>
    parseExpression(Parser::U32Iterator& iter);

    bool 
    parseInstant
    (
      Parser::U32Iterator& begin,
      const Parser::U32Iterator& end
    );

    //what is the input?
    uuid
    addExpr();

    bool
    parse_header(const u32string& s);

    void
    loadLibrary(const u32string& s);

    Parser::Header&
    header();

    const Tree::Expr&
    lastExpression() const;

    void
    go();

    private:
    //definitions of Equations
    typedef std::unordered_map<u32string, VariableWS*> DefinitionMap;

    //output hyperdatons
    typedef std::unordered_map<u32string, OutputHD*> OutputHDMap;
    //input hyperdatons
    typedef std::unordered_map<u32string, InputHD*> InputHDMap;

    //uuid to string
    typedef std::unordered_map<uuid, u32string, boost::hash<uuid>> 
      UUIDStringMap;

    // OPTYPE, ATL_SYMBOL
    typedef std::tuple<uuid, uuid> UnaryUUIDs;

    // OPTYPE, ATL_SYMBOL, ASSOC, PREC
    typedef std::tuple<uuid, uuid, uuid, uuid> BinaryUUIDs;

    //set of uuids
    typedef std::unordered_set<uuid, boost::hash<uuid>> UUIDHashSet;

    //uuids of set of uuids of unary operator equations
    typedef std::unordered_map<uuid, UnaryUUIDs, boost::hash<uuid>> 
      UnaryUUIDSet;

    //uuids of set of uuids of binary operator equations
    typedef std::unordered_map<uuid, BinaryUUIDs, boost::hash<uuid>>
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
    void
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
      DefinitionMap& declarations
    );

    Constant
    addDeclInternal(const Parser::Equation& eqn, DefinitionMap& declarations);

    void
    setDefaultContext();

    DefinitionMap m_equations;
    DefinitionMap m_assignments;

    //maps of string to hds and the hds uuids
    OutputHDMap m_outputHDs;
    UUIDStringMap m_outputUUIDs;

    InputHDMap m_inputHDs;

    //input and output hd declarations, for now just have the valid range
    std::unordered_map<u32string, Tuple> m_outputHDDecls;
    std::unordered_map<u32string, Tuple> m_inputHDDecls;

    //the uuid generator
    boost::uuids::basic_random_generator<boost::mt19937>
    m_uuid_generator;

    //---- the sets of all the uuids of objects ----

    //the uuids of the dimensions
    UUIDHashSet m_dimension_uuids;

    //the uuids of the unary operators
    UnaryUUIDSet m_unop_uuids;

    //the uuids of the binary operators
    BinaryUUIDSet m_binop_uuids;

    //registries
    type_index m_nextTypeIndex;
    ObjectRegistry<u32string, decltype(m_nextTypeIndex)> m_typeRegistry;
    DimensionTranslator m_dimTranslator;

    //std::map<u32string, size_t> builtin_name_to_index;

    //give ourselves a function registry which can register functions of
    //up to ten arguments
    FunctionRegistry<MAX_FUNCTION_PARAMETERS> m_functions;

    Tuple m_defaultk;
    size_t m_time;
    Translator *m_translator;

    friend class detail::InputHDWS;

    public:

    const Tuple&
    getDefaultContext()
    {
      return m_defaultk;
    }

    struct IdentifierLookup
    {
      IdentifierLookup(DefinitionMap& identifiers)
      : m_identifiers(identifiers)
      {
      }

      WS*
      lookup(const u32string& name) const
      {
        auto r = m_identifiers.find(name);
        if (r != m_identifiers.end())
        {
          return r->second;
        }
        else
        {
          return nullptr;
        }
      }

      private:
      DefinitionMap& m_identifiers;
    };

    IdentifierLookup lookupIdentifiers()
    {
      return IdentifierLookup(m_equations);
    }

    template <size_t N>
    auto
    lookupFunction(const u32string& name)
      -> decltype(m_functions.lookupFunction<N>(name))
    {
      return m_functions.lookupFunction<N>(name);
    }

    template <typename... Args>
    void
    registerFunction
    (
      const u32string& name, 
      std::function<Constant(Args... args)> f
    )
    {
      m_functions.registerFunction(name, f);
    }
  };

  Constant hash(const Constant& dimension, const Tuple& context);
  Constant hash(size_t dimension, const Tuple& context);
}

#endif // SYSTEM_HPP_INCLUDED
