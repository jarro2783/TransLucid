/* System hyperdaton.
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

#include <tl/types.hpp>
#include <tl/dimtranslator.hpp>
#include <tl/equation.hpp>

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

    typedef std::map<u32string, HD*> IOList;

    void
    addOutput(const IOList& output);

    void
    addInput(const IOList& input);

    void
    addDemand(const u32string& id, const EquationGuard& guard);

    void
    tick();

    private:
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

    template <typename T>
    HD*
    buildConstantHD(size_t index);

    void
    addDimensionSymbol(const u32string& s);

    std::map<u32string, size_t> builtin_name_to_index;
  };

  TypedValue hash(const TypedValue& dimension, const Tuple& context);
  TypedValue hash(size_t dimension, const Tuple& context);
}

#endif // INTERPRETER_HPP_INCLUDED
