/* Equations (ident = expr)
   Copyright (C) 2009, 2010, 2011 Jarryd Beck and John Plaice

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

#include <tl/ast_fwd.hpp>
#include <tl/exception.hpp>
#include <tl/bestfit.hpp>
#include <tl/workshop.hpp>
#include <tl/types.hpp>
#include <tl/uuid.hpp>

#include <list>
#include <memory>

namespace TransLucid
{
  class System;

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
  class GuardWS
  {
    public:

    /**
     * @brief Constructs a guard from an AST.
     *
     * Specifies the AST to use for the guard.
     **/
    GuardWS(WS* g, WS* b);

    /**
     * @brief Creates a guard with no AST.
     *
     * There are no user dimensions in the guard. System imposed
     * dimensions can still be added.
     **/
    GuardWS()
    : m_guard(nullptr), m_boolean(nullptr), m_compiled(true), 
      m_system(nullptr)
    , m_priority(0)
    {
    }

    GuardWS(const Tuple& t)
    : m_compiled(true), m_system(nullptr), m_priority(0)
    {
       for (Tuple::const_iterator iter = t.begin();
          iter != t.end();
          ++iter)
       {
          addDimension(iter->first, iter->second);
       }
    }

    GuardWS(const GuardWS&);

    ~GuardWS() = default;

    GuardWS& operator=(const GuardWS&);

    /**
     * @brief Determines if there are any dimensions in the guard.
     *
     * @return false if there are no user or system imposed dimensions,
     * true otherwise.
     **/
    operator bool() const
    {
       return 
          m_guard != nullptr
       || m_boolean != nullptr
       || m_dimConstConst.size() != 0
       || m_dimConstNon.size() != 0
       || m_dimNonConst.size() != 0
       || m_dimNonNon.size() != 0
       ;
    }

    /**
     * @brief Evaluate the guard.
     *
     * Returns a tuple of the dimensions and the evaluated AST.
     **/
    template <typename... Delta>
    std::pair<bool, Tuple>
    evaluate(Context& k, Delta&&... delta) const;

    /**
     * @brief Adds a system imposed dimension.
     *
     * The system can add dimensions to the guard which the user can't
     * change.
     **/
    void
    addDimension(size_t dim, const Constant& v)
    {
       m_dimConstConst[dim] = v;
    }

    WS*
    boolean() const
    {
       return m_boolean.get();
    }

    int
    priority() const
    {
      return m_priority;
    }

    //after a call to evaluate, these are reset to whatever demands evaluate
    //produced
    const std::vector<dimension_index>&
    demands() const
    {
      return m_demands;
    }

    private:

    void
    compile() const;

    std::shared_ptr<WS> m_guard;
    std::shared_ptr<WS> m_boolean;

    mutable std::map<dimension_index, Constant> m_dimConstConst;
    mutable std::map<dimension_index, WS*> m_dimConstNon;

    mutable std::map<WS*, Constant> m_dimNonConst;
    mutable std::map<WS*, WS*> m_dimNonNon;

    //have we compiled the guard?
    mutable bool m_compiled;

    mutable bool m_onlyConst;

    mutable System* m_system;

    mutable int m_priority;

    mutable std::vector<dimension_index> m_demands;
  };

  class VariableWS;

  typedef std::map<u32string, VariableWS*> VariableMap;

  class EquationWS : public WS
  {
    public:
    EquationWS(const u32string& name, const GuardWS& valid, WS* h,
      int provenance);

    EquationWS();

    ~EquationWS();

    const u32string&
    name() const
    {
       return m_name;
    }

    const GuardWS&
    validContext() const
    {
       return m_validContext;
    }

    operator bool() const
    {
       return m_h != 0;
    }

    WS*
    equation() const
    {
       return m_h.get();
    }

    const uuid&
    id() const
    {
      return m_id;
    }

    void
    del(size_t time);

    Constant
    operator()(Context& k)
    {
      return (*m_h)(k);
    }

    Constant
    operator()(Context& kappa, Context& delta)
    {
      return (*m_h)(kappa, delta);
    }

    int
    provenance() const
    {
      return m_provenance;
    }

    int
    priority() const
    {
      return m_priority;
    }

    int
    endTime() const
    {
      return m_endTime;
    }

    private:
    u32string m_name;
    GuardWS m_validContext;
    std::shared_ptr<WS> m_h;
    uuid m_id;
    Tree::Expr* m_ast;
    int m_provenance;
    int m_endTime;
    int m_priority;
  };

  //represents all definitions of a variable, is responsible for
  //JIT and best fitting
  class VariableWS : public WS
  {
    public:
    typedef std::map<uuid, EquationWS> UUIDEquationMap;
    typedef std::tuple<Tuple, VariableWS::UUIDEquationMap::const_iterator> 
      ApplicableTuple;
    typedef std::vector<ApplicableTuple> applicable_list;

    VariableWS(const u32string& name, System& system);
    
    ~VariableWS();

    virtual Constant
    operator()(Context& k);

    Constant
    operator()(Context& kappa, Context& delta);

    template <typename... Delta>
    Constant
    bestfit(const applicable_list& applicable, Context& k, Delta&&... delta);

    #if 0
    virtual uuid
    addExpr(const Tuple& k, WS* h)
    {
      return addExprInternal(k, h).first;
    }
    #endif

    uuid
    addEquation(EquationWS* e, size_t time);

    uuid
    addEquation
    (
      const u32string& name, 
      GuardWS guard, 
      WS* e, 
      size_t time
    );

    virtual bool 
    delexpr(uuid id, size_t time);

    virtual bool 
    replexpr(uuid id, size_t time, const GuardWS& guard, WS* expr);

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

    VariableMap m_variables;

    private:

    typedef std::list<std::pair<int, UUIDEquationMap::iterator>> 
      ProvenanceList;

    typedef std::map<int, ProvenanceList> PriorityList;

    //maps uuid to variables so that we know which child owns the equation
    //belonging to a uuid
    typedef std::map<uuid, VariableWS*> UUIDVarMap;

    UUIDEquationMap m_equations;

    u32string m_name;
    System& m_system;
    bool storeuuid;

    BestFittable m_bestFit;
    CompileBestFit *m_compileBestFit;

    PriorityList m_priorityVars;
  };

  class ConditionalBestfitWS : public WS
  {
    public:
    template <typename String, typename Enable = 
      std::enable_if<std::is_same<String, u32string>::value>
    >
    ConditionalBestfitWS(String&& name, System& system)
    {
      m_var = new VariableWS(U"fun_" + name, system);
    }

    ConditionalBestfitWS(const ConditionalBestfitWS&) = delete;
    ConditionalBestfitWS(ConditionalBestfitWS&) = delete;

    ~ConditionalBestfitWS()
    {
      delete m_var;
    }

    Constant
    operator()(Context& k);

    Constant
    operator()(Context& kappa, Context& delta);

    uuid
    addEquation
    (
      const u32string& name, 
      GuardWS guard, 
      WS* e, 
      size_t time
    )
    {
      return m_var->addEquation(name, guard, e, time);
    }

    private:
    VariableWS *m_var;
  };
};

#endif // EQUATION_HPP_INCLUDED
