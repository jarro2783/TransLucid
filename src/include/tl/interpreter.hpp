/* TODO: Give a descriptor.
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

#ifndef INTERPRETER_HPP_INCLUDED
#define INTERPRETER_HPP_INCLUDED

#include <tl/library.hpp>
#include <tl/types.hpp>
#include <tl/evaluator.hpp>
#include <tl/dimtranslator.hpp>
#include <boost/foreach.hpp>
//#include <tl/parser_fwd.hpp>
#include <deque>
#include <boost/assign.hpp>
#include <tl/cache.hpp>
#include <tl/equation.hpp>
#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  /**
   * @brief Interpreter base class.
   *
   * Holds all the data necessary for an interpreter.
   **/
  class Interpreter : public Variable
  {
    public:

    Interpreter();
    virtual ~Interpreter();

    typedef std::map<u32string, HD*> IOList;

    void
    addOutput(const IOList& output);

    void
    addInput(const IOList& input);

    void
    addDemand(const u32string& id, const EquationGuard& guard);

    void
    tick();

    /**
     * @brief Loads a library.
     *
     * Initialises the library which will add its types and operations
     * to the type registry.
     **/
    void
    loadLibrary(const u32string& name)
    {
      m_lt.loadLibrary(name, this);
    }

    /**
     * @brief Adds a search path for loading libraries.
     *
     * When a library is loaded the paths added here will be searched
     * for the library in the order they are added.
     **/
    void
    addLibrarySearchPath(const u32string& name)
    {
      m_lt.addSearchPath(name);
    }

    void
    registerEquation
    (const u32string& name, const Tuple& validContext, AST::Expr* e);

    //TaggedValue
    //operator()(const Tuple& k);

    //void
    //addExpr(const Tuple& k, HD* h);

    private:
    Libtool m_lt;
    TypeRegistry m_types;
    DimensionTranslator m_dimTranslator;

    size_t m_time;

    IOList m_outputs;
    IOList m_inputs;

    typedef std::map<u32string, EquationGuard> DemandStore;
    DemandStore m_demands;

    //initialises the type indexes
    void
    init_types();

    VariableMap m_variables;

    //adds to id with remaining in the id dimension
    //void
    //addToVariable
    //(
    //  const u32string& id,
    //  const u32string& remaining,
    //  const Tuple& k, HD* e
    //);
    //adds to id removing id from the context
    //void
    //addToVariable(const u32string& id, const Tuple& k, HD* e);
    //does the actual add
    //void
    //addToVariableActual(const u32string& id, const Tuple& k, HD* e);

    template <typename T>
    HD*
    buildConstantHD(size_t index);

    void
    addDimensionSymbol(const u32string& s);

    std::map<u32string, size_t> builtin_name_to_index;

    protected:

    bool m_verbose;

    void cleanupParserObjects();
  };

  TypedValue hash(const TypedValue& dimension, const Tuple& context);
  TypedValue hash(size_t dimension, const Tuple& context);
}

#endif // INTERPRETER_HPP_INCLUDED
