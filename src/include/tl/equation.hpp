/* Equations (ident = expr)
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

#ifndef EQUATION_HPP_INCLUDED
#define EQUATION_HPP_INCLUDED

#include <tl/types.hpp>
#include <boost/shared_ptr.hpp>
#include <tl/exception.hpp>
#include <list>
#include <deque>
#include <tl/hyperdaton.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <tl/bestfit.hpp>

namespace TransLucid
{
  class SystemHD;

  class InvalidGuard : public Exception
  {
  };

  /**
   * @brief Represents the guard of an equation.
   *
   * The guard is the bit between the @ and the = in
   * eqn @ [dim : E, ...] = expr.
   * The system can impose elements of the guard, it is an error for
   * the user to specify those ones too.
   **/
  class GuardHD
  {
    public:

    /**
     * @brief Constructs a guard from an AST.
     *
     * Specifies the AST to use for the guard.
     **/
    GuardHD(HD* g, HD* b)
    : m_guard(g), m_boolean(b)
    {
    }

    /**
     * @brief Creates a guard with no AST.
     *
     * There are no user dimensions in the guard. System imposed
     * dimensions can still be added.
     **/
    GuardHD()
    : m_guard(0), m_boolean(0)
    {
    }

    GuardHD(const Tuple& t)
    : m_guard(0), m_boolean(0)
    {
       for (Tuple::const_iterator iter = t.begin();
          iter != t.end();
          ++iter)
       {
          addDimension(iter->first, iter->second);
       }
    }

    /**
     * @brief Determines if there are any dimensions in the guard.
     *
     * @return false if there are no user or system imposed dimensions,
     * true otherwise.
     **/
    operator bool() const
    {
       return m_guard != 0 || m_dimensions.size() != 0 || m_boolean != 0;
    }

    /**
     * @brief Evaluate the guard.
     *
     * Returns a tuple of the dimensions and the evaluated AST,
     * this includes all the system imposed dimensions.
     * @throw InvalidGuard when the user has specified a system imposed
     * dimension.
     **/
    Tuple
    evaluate(const Tuple& k) const;

    /**
     * @brief Adds a system imposed dimension.
     *
     * The system can add dimensions to the guard which the user can't
     * change.
     **/
    void
    addDimension(size_t dim, const Constant& v)
    {
       m_dimensions[dim] = v;
    }

    HD*
    boolean() const
    {
       return m_boolean;
    }

    private:
    HD* m_guard;
    HD* m_boolean;
    std::map<size_t, Constant> m_dimensions;
  };

  class VariableHD;

  typedef std::map<u32string, VariableHD*> VariableMap;

  class EquationHD : public HD
  {
    public:
    EquationHD(const u32string& name, const GuardHD& valid, HD* h)
    : m_name(name), m_validContext(valid), m_h(h),
    //m_id(boost::uuids::random_generator()())
    //m_id(boost::uuids::nil_generator()())
    //m_id(boost::uuids::basic_random_generator<boost::rand48>()())
    m_id(m_generator())
    {
    }

    EquationHD()
    : m_h(0), m_id(boost::uuids::nil_generator()())
    {
    }

    const u32string&
    name() const
    {
       return m_name;
    }

    const GuardHD&
    validContext() const
    {
       return m_validContext;
    }

    operator bool() const
    {
       return m_h != 0;
    }

    HD*
    equation() const
    {
       return m_h;
    }

    const uuid&
    id() const
    {
      return m_id;
    }

    void
    del(size_t time);

    TaggedConstant
    operator()(const Tuple& k)
    {
      return (*m_h)(k);
    }

    private:
    u32string m_name;
    GuardHD m_validContext;
    HD* m_h;
    boost::uuids::uuid m_id;
    size_t m_validStart;
    size_t m_validEnd;

    static boost::uuids::basic_random_generator<boost::mt19937>
    m_generator;
  };

  //represents all definitions of a variable, is responsible for
  //JIT and best fitting
  class VariableHD : public HD
  {
    public:

    VariableHD(const u32string& name, HD* system);
    
    ~VariableHD();

    TaggedConstant operator()(const Tuple& k);

    uuid
    addExpr(const Tuple& k, HD* h)
    {
      return addExprInternal(k, h).first;
    }

    bool delexpr(uuid id, size_t time);

    bool replexpr(uuid id, size_t time, const GuardHD& guard, HD* expr);

    protected:

    typedef std::map<uuid, EquationHD> Equations;

    std::pair<uuid, Equations::iterator>
    addToVariableActual(const u32string& id, const Tuple& k, HD* h);

    private:

    //maps uuid to variables so that we know which child owns the equation
    //belonging to a uuid
    typedef std::map<uuid, VariableHD*> UUIDVarMap;

    std::pair<uuid, Equations::iterator>
    addExprInternal(const Tuple& k, HD* h);

    std::pair<uuid, Equations::iterator>
    addExprActual(const Tuple& k, HD* e);

    bool equationValid(const EquationHD& e, const Tuple& k);

    UUIDVarMap m_uuidVars;
    Equations m_equations;
    VariableMap m_variables;

    u32string m_name;
    HD* m_system;
    bool storeuuid;

    BestFittable m_bestFit;
    CompileBestFit *m_compileBestFit;
  };
};

#endif // EQUATION_HPP_INCLUDED
