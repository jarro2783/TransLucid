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

//#include <tl/ast.hpp>
#include <tl/exception.hpp>
#include <tl/bestfit.hpp>
#include <tl/hyperdaton.hpp>
#include <tl/types.hpp>

#include <deque>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/uuid/uuid_generators.hpp>

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
    : m_guard(g), m_boolean(b), m_timeStart(0), m_timeEnd(0)
    {
    }

    /**
     * @brief Creates a guard with no AST.
     *
     * There are no user dimensions in the guard. System imposed
     * dimensions can still be added.
     **/
    GuardHD()
    : m_guard(0), m_boolean(0), m_timeStart(0), m_timeEnd(0)
    {
    }

    GuardHD(const Tuple& t)
    : m_guard(0), m_boolean(0), m_timeStart(0), m_timeEnd(0)
    {
       for (Tuple::const_iterator iter = t.begin();
          iter != t.end();
          ++iter)
       {
          addDimension(iter->first, iter->second);
       }
    }

    GuardHD(const GuardHD&);

    ~GuardHD()
    {
      delete m_timeStart;
      delete m_timeEnd;
    }

    GuardHD& operator=(const GuardHD&);

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

    void 
    setTimeStart(mpz_class time)
    {
      m_timeStart = new mpz_class(time);
    }

    void 
    setTimeEnd(mpz_class time)
    {
      m_timeEnd = new mpz_class(time);
    }

    private:
    HD* m_guard;
    HD* m_boolean;
    std::map<size_t, Constant> m_dimensions;

    mpz_class *m_timeStart;
    mpz_class *m_timeEnd;
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

    static boost::uuids::basic_random_generator<boost::mt19937>
    m_generator;
  };

  //represents all definitions of a variable, is responsible for
  //JIT and best fitting
  class VariableHD : public HD
  {
    public:
    typedef std::map<uuid, EquationHD> UUIDEquationMap;

    VariableHD(const u32string& name, HD* system);
    
    ~VariableHD();

    virtual TaggedConstant 
    operator()(const Tuple& k);

    uuid
    addEquation(EquationHD* e, size_t time);

    uuid
    addEquation
    (
      const u32string& name, 
      GuardHD guard, 
      HD* e, 
      size_t time
    );

    virtual bool 
    delexpr(uuid id, size_t time);

    virtual bool 
    replexpr(uuid id, size_t time, const GuardHD& guard, HD* expr);

    /**
     * The equations belonging directly to this variable. Returns the map of
     * UUIDs to equations for the current variable.
     * @return A map of UUID to the current variable's equations.
     */
    const UUIDEquationMap& equations() const
    {
      return m_equations;
    }

    protected:

    std::pair<uuid, UUIDEquationMap::iterator>
    addToVariableActual(const u32string& id, const Tuple& k, HD* h);
    VariableMap m_variables;

    private:

    //maps uuid to variables so that we know which child owns the equation
    //belonging to a uuid
    typedef std::map<uuid, VariableHD*> UUIDVarMap;

    std::pair<uuid, UUIDEquationMap::iterator>
    addExprInternal(const Tuple& k, HD* h);

    std::pair<uuid, UUIDEquationMap::iterator>
    addExprActual(const Tuple& k, HD* e);

    bool equationValid(const EquationHD& e, const Tuple& k);

    UUIDEquationMap m_equations;

    u32string m_name;
    HD* m_system;
    bool storeuuid;

    BestFittable m_bestFit;
    CompileBestFit *m_compileBestFit;
  };
};

#endif // EQUATION_HPP_INCLUDED
