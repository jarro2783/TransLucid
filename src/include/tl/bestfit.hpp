/* Equations (ident = expr)
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

#ifndef BESTFIT_HPP_INCLUDED
#define BESTFIT_HPP_INCLUDED

#include <tl/context.hpp>
#include <tl/parser_api.hpp>
#include <tl/types.hpp>
#include <tl/uuid.hpp>
#include <tl/workshop.hpp>

#include <list>
#include <unordered_map>

/**
 * @file bestfit.hpp
 * The optimisation of best fitting.
 */

namespace TransLucid 
{
  class VariableWS;
  class EquationWS;

  /**
   * Computes the best fit for an equation.
   * A best fitting interface which any best fitter will inherit from.
   */
  class BestFit
  {
    public:
    virtual ~BestFit() {}

    /**
     * Find the best fit and evaluate.
     */
    virtual TaggedConstant operator()(const Tuple& k) = 0;
  };

  class BestFittable
  {
    public:
    BestFittable()
    : m_bestFit(nullptr)
    {
    }

    //returns the old best fit
    BestFit* setBestFit(BestFit* b)
    {
      BestFit *old = m_bestFit;
      m_bestFit = b;
      return old;
    }

    TaggedConstant operator()(Tuple& k)
    {
      return (*m_bestFit)(k);
    }

    private:
    BestFit* m_bestFit;
  };

  /**
   * Compiles a new best fit. This is the just in time compilation
   * of a best fit. Before the first best fit, the best fit will be of
   * this type. So first it compiles the best fit and then runs the
   * appropriate best fit.
   */
  class CompileBestFit : public BestFit
  {
    public:
    //CompileBestFit(EquationWS& e, VariableWS& v);

    TaggedConstant operator()(const Tuple& k);

    private:
    ~CompileBestFit() {}

    BestFittable* m_bestFittable;
  };

  /**
   * The dumb best fit. Tries to find all definitions and determines
   * which is applicable.
   */
  class BruteForceBestFit : public BestFit
  {
    public:
    TaggedConstant operator()(Tuple& k);
  };

  class SingleDefinitionBestFit : public BestFit
  {
  };

  class EquationDefinition
  {
    public:

    EquationDefinition
    (
      uuid id,
      int start,
      int end
    )
    : m_uuid(id)
    , m_start(start)
    , m_end(end)
    {
    }

    uuid
    id() const
    {
      return m_uuid;
    }

    int
    start() const
    {
      return m_start;
    }

    int 
    end() const
    {
      return m_end;
    }

    void
    setEnd(int end)
    {
      m_end = end;
    }

    void
    setRaw(Parser::RawInput raw)
    {
      m_raw = std::make_shared<Parser::RawInput>(raw);
    }

    void
    setParsed(Parser::Line parsed)
    {
      m_parsed = std::make_shared<Parser::Line>(parsed);
    }

    private:
    uuid m_uuid;
    int m_start;
    int m_end;
    std::shared_ptr<Parser::RawInput> m_raw;
    std::shared_ptr<Parser::Line> m_parsed;

    public:

    decltype(m_raw)
    raw() const
    {
      return m_raw;
    }

    decltype(m_parsed)
    parsed() const
    {
      return m_parsed;
    }
  };

  class DefinitionGrouper
  {
    public:
    virtual Tree::Expr
    group(const std::list<EquationDefinition>&) = 0;
  };

  class BestfitGroup
  {
    public:

    BestfitGroup(DefinitionGrouper* grouper, System& system)
    : m_grouper(grouper)
    , m_system(system)
    , m_parsed(0)
    {
    }

    void
    addEquation
    (
      uuid id,
      Parser::RawInput input,
      int time
    )
    {
      m_definitions.push_back(EquationDefinition{id, time, -1});
      m_definitions.back().setRaw(input);
      change(time);
    }

    void
    addEquation
    (
      uuid id,
      Parser::Line definition,
      int time
    )
    {
      m_definitions.push_back(EquationDefinition{id, time, -1});
      m_definitions.back().setParsed(definition);
      change(time);
    }

    bool
    del(uuid id, int time)
    {
      auto iter = m_uuids.find(id);

      if (iter == m_uuids.end())
      {
        return false;
      }

      auto last = iter->second.back();

      //if it's already been deleted then don't try to delete it again
      if (m_definitions[last].end() != -1)
      {
        return false;
      }
      else
      {
        m_definitions[last].setEnd(time);
      }

      change(time);

      return true;
    }

    bool
    repl(uuid id, size_t time, Parser::Line line)
    {
      //TODO implement me
      return false;
    }

    Constant
    operator()(Context& k);

    private:

    void
    parse(Context& k);

    void
    adduuid(uuid id, int time)
    {
      auto iter = m_uuids.find(id);

      if (iter == m_uuids.end())
      {
        m_uuids.insert(
        {
          id, 
          std::list<size_t>{m_definitions.size() - 1}
        });
      }
      else
      {
        iter->second.push_back(m_definitions.size() - 1);
      }
    }

    void
    change(int time)
    {
      if (m_changes.empty() || m_changes.back() < time)
      {
        m_changes.push_back(time);
      }
    }

    void
    compile(Context& k);

    //compiles all the equations valid for the instant time
    Tree::Expr
    compileInstant(int time);

    std::shared_ptr<WS>
    compileExpression(const Tree::Expr& expr);

    Constant
    evaluate(Context& k);

    struct CompiledDefinition
    {
      int start;
      int end;
      std::shared_ptr<WS> evaluator;
    };

    DefinitionGrouper* m_grouper;

    System& m_system;

    std::map<uuid, std::list<size_t>> m_uuids;
    std::vector<EquationDefinition> m_definitions;

    //record when things change, and recompile the whole lot
    //every time a change occurs
    std::vector<int> m_changes;

    //a list of compiled definitions in order of valid times
    std::vector<CompiledDefinition> m_evaluators;

    size_t m_parsed;
  };

  /**
   * @brief Represents the guard of an equation.
   *
   * The tuple and boolean parts of an equation.
   **/
  class EquationGuard
  {
    public:

    /**
     * @brief Constructs a guard from an AST.
     *
     * Specifies the AST to use for the guard.
     **/
    EquationGuard(WS* g, WS* b);

    /**
     * @brief Creates a guard with no AST.
     *
     * There are no user dimensions in the guard. System imposed
     * dimensions can still be added.
     **/
    EquationGuard()
    : m_guard(nullptr), m_boolean(nullptr), m_compiled(true), 
      m_system(nullptr)
    , m_priority(0)
    {
    }

    EquationGuard(const Tuple& t)
    : m_compiled(true), m_system(nullptr), m_priority(0)
    {
       for (Tuple::const_iterator iter = t.begin();
          iter != t.end();
          ++iter)
       {
          addDimension(iter->first, iter->second);
       }
    }

    EquationGuard(const EquationGuard&);

    ~EquationGuard() = default;

    EquationGuard& operator=(const EquationGuard&);

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


  class CompiledEquation : public WS
  {
    public:
    CompiledEquation
    (
      const EquationGuard& valid, 
      std::shared_ptr<WS> evaluator,
      int provenance
    );

    CompiledEquation();

    ~CompiledEquation();

    const EquationGuard&
    validContext() const
    {
       return m_validContext;
    }

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

    private:
    EquationGuard m_validContext;
    std::shared_ptr<WS> m_h;
    int m_provenance;
    int m_priority;
  };

  class ConditionalBestfitWS : public WS
  {
    public:
    typedef std::vector<CompiledEquation> Equations;
    ConditionalBestfitWS(Equations e)
    : m_equations(e)
    {
    }

    ConditionalBestfitWS(const ConditionalBestfitWS&) = delete;
    ConditionalBestfitWS(ConditionalBestfitWS&) = delete;

    ~ConditionalBestfitWS()
    {
    }

    Constant
    operator()(Context& k);

    Constant
    operator()(Context& kappa, Context& delta);

    private:
    typedef std::tuple<Tuple, Equations::iterator> 
      ApplicableTuple;
    typedef std::vector<ApplicableTuple> applicable_list;

    typedef std::list<std::pair<int, Equations::iterator>> 
      ProvenanceList;

    typedef std::map<int, ProvenanceList> PriorityList;

    template <typename... Delta>
    Constant
    bestfit(const applicable_list& applicable, Context& k, 
      Delta&&... delta);

    Equations m_equations;
    PriorityList m_priorityVars;
  };
}

#endif // BESTFIT_HPP_INCLUDED
