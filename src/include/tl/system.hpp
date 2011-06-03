/* System hyperdaton.
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
#include <tl/physicalhds.hpp>
#include <tl/parser_api.hpp>

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
  class System : public WS
  {
    public:

    System();
    ~System();

    //don't want to copy
    System(const System&) = delete;

    TaggedConstant
    operator()(const Tuple& k);

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

    //what is the input?
    uuid
    addExpr();

    //output hyperdatons, set of context
    void
    eval(const std::list<uuid>& exprs, PhysicalWS* out);

    WS*
    translate_expr(const u32string& s);

    std::list<std::pair<uuid, Parser::ParsedEquation>>
    translate_and_add_equation_set(const u32string& s);

    PTEquationVector
    translate_equation_set(const u32string& s);

    bool
    parse_header(const u32string& s);

    void
    loadLibrary(const u32string& s);

    Parser::Header&
    header();

    const Tree::Expr&
    lastExpression() const;

    private:
    DimensionTranslator m_dimTranslator;

    size_t m_time;

    //initialises the type indexes
    void
    init_types();

    template <typename T>
    WS*
    buildConstantWS(size_t index);

    void
    addDimensionSymbol(const u32string& s);

    void
    tick();

    std::map<u32string, size_t> builtin_name_to_index;

    std::map<u32string, VariableWS*> m_equations;

    //the uuid generator
    boost::uuids::basic_random_generator<boost::mt19937>
    m_uuid_generator;

    Translator *m_translator;
  };

  Constant hash(const Constant& dimension, const Tuple& context);
  Constant hash(size_t dimension, const Tuple& context);
}

#endif // SYSTEM_HPP_INCLUDED
