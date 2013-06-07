/* Assignments
   Copyright (C) 2012 Jarryd Beck

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

/**
 * @file assignment.cpp
 * Assignment declarations.
 */

#include <tl/assignment.hpp>
#include <tl/fixed_indexes.hpp>
#include <tl/system.hpp>
#include <tl/types/region.hpp>
#include <tl/types/tuple.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

namespace {
  //for every context in ctxts that is valid in k,
  //output the result of computation compute to out
  //at the moment we only know how to enumerate ranges, this could
  //become richer as we work out the type system better
  void
  enumerateContextSet
  (
    const Region& ctxts, 
    Context& k,
    WS& compute,
    OutputHD* out
  )
  {
    //Tuple variance = out->variance();
    Region variance;

    //all the ranges in order
    std::vector<Range> limits;

    //the current value of each range dimension
    std::vector<std::pair<dimension_index, mpz_class>> current;

    //the context to evaluate in
    Context evalContext(k);

    //determine which dimensions are ranges
    for (const auto& v : ctxts)
    {
      if (v.second.second.index() == TYPE_INDEX_RANGE && 
          v.second.first == Region::Containment::IN)
      {
        const Range& r = Types::Range::get(v.second.second);

        if (r.lower() == nullptr || r.upper() == nullptr)
        {
          //std::cerr << "Error: infinite bounds in demand, dimension " <<
          //  v.first << std::endl;
          throw "Infinite bounds in demand";
        }

        limits.push_back(r);
        current.push_back(std::make_pair(v.first, *r.lower()));
      }
      else
      {
        //if not a range then store it permanantly
        evalContext.perturb(v.first, v.second.second);
      }
    }

    //by doing it this way, even if there is no range, we still evaluate
    //everything once
    while (true)
    {
      ContextPerturber p(evalContext);
      for (const auto& v : current)
      {
        p.perturb(v.first, Types::Intmp::create(v.second));
      }

      //is the demand valid for the hyperdaton, and is the demand valid for
      //the current context
      if (regionApplicable(variance, evalContext) && k <= evalContext)
      {
        out->put(evalContext, compute(evalContext));
      }
      
      //then we increment the counters at the end
      auto limitIter = limits.begin();
      auto currentIter = current.begin();
      while (currentIter != current.end())
      {
        ++currentIter->second;
        if (!limitIter->within(currentIter->second))
        {
          currentIter->second = *limitIter->lower();
          ++currentIter;
          ++limitIter;
        }
        else
        {
          break;
        }
      }
      
      if (currentIter == current.end())
      {
        break;
      }
    }
  }

}

void
Assignment::evaluate
(
  System& s,
  Context& k
)
{
  auto hd = s.getOutputHD(m_name);

  if (hd == nullptr)
  {
    //std::cerr << "warning: output hyperdaton \"" << ident.first
    //  << "\" doesn't exist" << std::endl;
    return;
  }

  Context theContext = k;

  //theContext.perturb(DIM_TIME, Types::Intmp::create(m_time));

  //this needs to be way better
  //for a start: only look at demands for the current time
  //auto equations = ident.second->equations();
  for (auto& assign : m_definitions)
  {
    //const Tuple& constraint = m_outputHDDecls.find(ident.first)->second;
    const auto& guard = assign.guardWS;

    if (guard)
    {
      auto ctxts = (*guard)(theContext);

      if (ctxts.index() == TYPE_INDEX_REGION)
      {
        //the demand could have ranges, so we need to enumerate them
        enumerateContextSet(Types::Region::get(ctxts), theContext, 
          *assign.bodyWS, hd);
      }
    }
  }
}

}
