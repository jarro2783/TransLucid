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
    group(const std::list<Parser::Line>&) = 0;
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
      if (m_changes.empty() || m_changes.back() != time)
      {
        m_changes.push_back(time);
      }

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

      return true;
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
      if (m_changes.empty() || m_changes.back() != time)
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
}

#endif // BESTFIT_HPP_INCLUDED
