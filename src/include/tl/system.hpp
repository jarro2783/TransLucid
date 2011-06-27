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
    parseLine(Parser::U32Iterator& begin);

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
    typedef std::unordered_map<u32string, VariableWS*> DefinitionMap;
    typedef std::unordered_map<u32string, OutputHD*> OutputHDMap;

    typedef std::unordered_map<uuid, u32string, boost::hash<uuid>> 
      UUIDStringMap;

    // OPTYPE, ATL_SYMBOL
    typedef std::tuple<uuid, uuid> UnaryHashes;

    // OPTYPE, ATL_SYMBOL, ASSOC, PREC
    typedef std::tuple<uuid, uuid, uuid, uuid> BinaryHashes;

    typedef std::unordered_set<uuid, boost::hash<uuid>> UUIDHashSet;
    typedef std::unordered_map<uuid, UnaryHashes, boost::hash<uuid>> 
      UnaryHashSet;
    typedef std::unordered_map<uuid, BinaryHashes, boost::hash<uuid>>
      BinaryHashSet;

    //initialises the type indexes
    void
    init_types();

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

    //the uuid generator
    boost::uuids::basic_random_generator<boost::mt19937>
    m_uuid_generator;

    //---- the sets of all the uuids of objects ----
    UUIDHashSet m_dimension_uuids;
    UnaryHashSet m_unop_uuids;
    BinaryHashSet m_binop_uuids;

    DimensionTranslator m_dimTranslator;

    type_index m_nextTypeIndex;
    ObjectRegistry<u32string, decltype(m_nextTypeIndex)> m_typeRegistry;

    size_t m_time;
    Translator *m_translator;
    std::map<u32string, size_t> builtin_name_to_index;

    //give ourselves a function registry which can register functions of
    //up to ten arguments
    FunctionRegistry<10> m_functions;

    Tuple m_defaultk;

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
  };

  Constant hash(const Constant& dimension, const Tuple& context);
  Constant hash(size_t dimension, const Tuple& context);
}

#endif // SYSTEM_HPP_INCLUDED
