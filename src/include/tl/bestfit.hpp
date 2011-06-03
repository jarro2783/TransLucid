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

#ifndef BESTFIT_HPP_INCLUDED
#define BESTFIT_HPP_INCLUDED

#include <tl/types.hpp>

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
    : m_bestFit(0)
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
}

#endif // BESTFIT_HPP_INCLUDED
