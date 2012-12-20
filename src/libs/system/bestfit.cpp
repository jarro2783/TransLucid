/* Equations (ident = expr)
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

#include <tl/bestfit.hpp>
#include <tl/eval_workshops.hpp>
#include <tl/output.hpp>
#include <tl/tree_printer.hpp>
#include <tl/types/demand.hpp>
#include <tl/types/region.hpp>
#include <tl/types/tuple.hpp>
#include <tl/types/union.hpp>
#include <tl/utility.hpp>
#include <tl/workshop_builder.hpp>

#include "tl/parser.hpp"

/**
 * @file bestfit.cpp
 * The implementation of bestfitting.
 */

static constexpr size_t LINEAR_SEARCH_MAX_SIZE = 7;

namespace TransLucid
{

namespace
{
  //is a a subset of b
  typedef bool (*IsSubsetFn)(const Constant& a, const Constant& b);  

  //returns a function that determines if the parameter is a subset of
  //the particular type that it handles
  typedef IsSubsetFn (*IsSubsetOf)(const Constant&);

  //several things are immediately false
  bool
  issubset_false(const Constant&, const Constant&)
  {
    return false;
  }

  bool
  constants_equal(const Constant& a, const Constant& b)
  {
    return a == b;
  }

  template <typename T>
  struct TupleLookup;

  template <>
  struct TupleLookup<Tuple>
  {
    Constant
    operator()(const Tuple& t, dimension_index dim)
    {
      auto iter = t.find(dim);

      if (iter == t.end())
      {
        return Types::Special::create(SP_DIMENSION);
      }
      else
      {
        return iter->second;
      }
    }
  };

  template <>
  struct TupleLookup<Context>
  {
    Constant
    operator()(const Context& k, dimension_index dim)
    {
      return k.lookup(dim);
    }
  };

  //there are several types that can only be compared with something
  //of the same type, instead of writing the code several times for
  //each, we will use templates.
  //In that case, constant equality works fine.
  template <type_index T>
  IsSubsetFn
  isSubsetAtomic(const Constant& c)
  {
    if (c.index() != T)
    {
      return &issubset_false;
    }
    else
    {
      return &constants_equal;
    }
  }

  //TODO: work out tuples and tuple sets
  //this currently returns false if the two are equal
  //we probably want it to be true, but in some cases we want it to
  //be false, so we have to work out what it all means
  bool
  tuple_subset_tuple(const Constant& sub, const Constant& super)
  {
    //std::cerr << "tuple subset in bestfit :)" << std::endl;
    //bool r = 
    return 
    tupleRefines
    (
      Types::Tuple::get(sub),
      Types::Tuple::get(super),
      true
    );

    //std::cerr << "refines = " << r << std::endl;
    //return r;
  }

  IsSubsetFn 
  subset_of_region(const Constant& c)
  {
    if (c.index() == TYPE_INDEX_TUPLE)
    {
      return [] (const Constant& a, const Constant& b) -> bool
      {
        //b is the region, a is a tuple
        return regionApplicable(Types::Region::get(b), Types::Tuple::get(a));
      };
    }
    else if (c.index() == TYPE_INDEX_REGION)
    {
      return [] (const Constant& a, const Constant& b) -> bool
      {
        return regionSubset(Types::Region::get(a), Types::Region::get(b), 
          true);
      };
    }

    return &issubset_false;
  }

  IsSubsetFn 
  subset_of_tuple(const Constant& c)
  {
    if (c.index() != TYPE_INDEX_TUPLE)
    {
      return &issubset_false;
    }
    else
    {
      return &tuple_subset_tuple;
    }
  }

  IsSubsetFn
  subset_of_range(const Constant& c)
  {
    //at the moment we can only do ranges of integers
    
    if (c.index() == TYPE_INDEX_INTMP)
    {
      //lambda functions to the rescue
      return [] (const Constant& a, const Constant& b)
        {
          //a is an int, b is a range
          return Types::Range::get(b).within(Types::Intmp::get(a));
        }
      ;
    }
    else if (c.index() == TYPE_INDEX_RANGE)
    {
      return [] (const Constant& a, const Constant& b)
        {
          //a is a range, b is a range
          return Types::Range::get(b).within(Types::Range::get(a));
        }
      ;
    }
    else
    {
      return &issubset_false;
    }
  }

  IsSubsetFn
  subset_of_type(const Constant& c)
  {
    return [] (const Constant& a, const Constant& b)
      {
        if (a.index() == TYPE_INDEX_TYPE)
        {
          return a == b;
        }
        //b is a type, a is anything
        return (a.index() == get_constant<type_index>(b));
      }
    ;
  }

  bool subset_union_constant(const Constant& a, const Constant& b)
  {
    //a is a union
    const UnionType& u = Types::Union::get(b);

    return u.contains(a);
  }

  bool subset_union_union(const Constant& a, const Constant& b)
  {
    return false;
  }

  IsSubsetFn
  subset_of_union(const Constant& c)
  {
    if (c.index() == TYPE_INDEX_UNION)
    {
      return &subset_union_union;
    }
    else
    {
      return &subset_union_constant;
    }
  }

  class TypeComparators
  {
    public:

    TypeComparators()
    {
      m_funs = new IsSubsetOf[NUM_FUNS]
        {
          &isSubsetAtomic<TYPE_INDEX_ERROR>,
          &isSubsetAtomic<TYPE_INDEX_BOOL>,
          &isSubsetAtomic<TYPE_INDEX_SPECIAL>,
          &isSubsetAtomic<TYPE_INDEX_INTMP>,
          &isSubsetAtomic<TYPE_INDEX_UCHAR>,
          &isSubsetAtomic<TYPE_INDEX_USTRING>,
          &isSubsetAtomic<TYPE_INDEX_FLOATMP>,
          &isSubsetAtomic<TYPE_INDEX_DIMENSION>,
          &subset_of_region,
          &subset_of_tuple,
          &subset_of_type,
          &subset_of_range,
          &subset_of_union
        };
    }

    ~TypeComparators()
    {
      delete [] m_funs;
    }

    IsSubsetOf
    getComparator(type_index t)
    {
      if (t < NUM_FUNS)
      {
        return m_funs[t];
      }
      else
      {
        return nullptr;
      }
    }

    private:
    static const int NUM_FUNS = TYPE_INDEX_UNION + 1;
    IsSubsetOf* m_funs;
  };

  static TypeComparators typeCompare;
}

//TODO finish this
//the choice of how to best fit will very much depend on properties of the
//system
TaggedConstant CompileBestFit::operator()(const Tuple& k)
{
  BestFit* b = nullptr;
  BestFit* old = m_bestFittable->setBestFit(b);
  if (old != this) {
  }

  return (*b)(k);
}

void
BestfitGroup::parse(Context& k)
{
  Parser::Parser p(m_system);

  while (m_parsed != m_definitions.size())
  {
    auto& definition = m_definitions.at(m_parsed);

    if (definition.raw() != nullptr)
    {
      auto& text = *definition.raw();

      Parser::U32Iterator ubegin(Parser::makeUTF32Iterator(text.text.begin()));
      Parser::U32Iterator uend(Parser::makeUTF32Iterator(text.text.end()));

      Parser::StreamPosIterator posbegin(ubegin, text.source, 
        text.line, text.character);
      Parser::StreamPosIterator posend(uend);

      Parser::LexerIterator lexbegin(posbegin, posend, k, 
        m_system.lookupIdentifiers());
      Parser::LexerIterator lexend = lexbegin.makeEnd();

      Parser::Line result;
      bool success = p.parse_decl(lexbegin, lexend, result);

      if (!success)
      {
        throw "parse error";
      }
      else
      {
        //if this is a repl declaration then we just want the actual
        //declaration
        auto repl = get<Parser::ReplDecl>(&result);

        if (repl != nullptr)
        {
          definition.setParsed(repl->repl);
        }
        else
        {
          definition.setParsed(result);
        }
      }
    }

    ++m_parsed;
  }
}

void
BestfitGroup::compile(Context& k)
{
  m_compiling = true;
  //std::cerr << "compiling " << m_name << std::endl;
  parse(k);

  //add in the extra definitions for that which has changed
  if (m_changes.size() > m_evaluators.size())
  {
    size_t change = m_evaluators.size();

    while (change != m_changes.size())
    {
      //the start is where that change was
      int start = m_changes[change];
      //there is no end
      int end = -1;

      if (change != 0)
      {
        //the previous should end one before the current time
        m_evaluators[change-1].end = m_changes[change] - 1;
      }

      //compile one group of definitions
      Tree::Expr expression = compileInstant(m_changes[change]);

      //make this into a workshop
      std::shared_ptr<WS> evaluator = compileExpression(expression, 
        m_definitions.front().getScope());

      m_evaluators.push_back(CompiledDefinition{start, end, evaluator});

      ++change;
    }
  }

  m_compiling = false;
}

std::shared_ptr<WS>
BestfitGroup::compileExpression(const Tree::Expr& expr, ScopePtr scope)
{
  //fixup the ast
  Tree::Expr fixed = m_system.fixupTreeAndAdd(expr, scope);

  //std::cerr << m_name << ": fixed up tree: " 
  //  << Printer::print_expr_tree(fixed, true)
  //  << ";;\n" << std::endl;

  //compile the tree into a workshop
  WorkshopBuilder compile(&m_system);
  std::shared_ptr<WS> ws(compile.build_workshops(fixed));

  return ws;
}

Tree::Expr
BestfitGroup::compileInstant(int time)
{
  //look for everything that is valid at time and compile it into one
  //expression

  std::list<EquationDefinition> valid;
  for (auto i = m_definitions.begin(); i != m_definitions.end(); 
    ++i)
  {
    if (i->start() <= time && (i->end() == -1 || i->end() < time))
    {
      valid.push_back(*i);
    }
  }

  return m_grouper->group(valid);
}

Constant
BestfitGroup::operator()(Context& k)
{
  if (m_compiling)
  {
    throw U"loop compiling BestfitGroup: " + m_name;
  }

  //if (m_parsed != m_definitions.size())
  if (m_changes.size() > m_evaluators.size())
  {
    try
    {
      compile(k);
    }
    catch (const Parser::ParseError& e)
    {
      std::cerr << "exception parsing: " << e.m_pos.file << ":" << e.m_pos.line 
                << ":" << e.m_pos.character << ":" << e.what() << std::endl;
      throw e;
    }
  }

  return evaluate(k);
}

Constant
BestfitGroup::operator()(Context& kappa, Context& delta)
{
  #ifdef CACHE_TODO
  #warning implement cache here
  #endif
  return Constant();
}

TimeConstant
BestfitGroup::operator()(Context& kappa, Delta& d, const Thread& w, size_t t)
{
  if (m_compiling)
  {
    throw U"loop compiling BestfitGroup: " + m_name;
  }

  //if (m_parsed != m_definitions.size())
  if (m_changes.size() > m_evaluators.size())
  {
    try
    {
      compile(kappa);
    }
    catch (const Parser::ParseError& e)
    {
      std::cerr << "exception parsing: " << e.m_pos.file << ":" << e.m_pos.line 
                << ":" << e.m_pos.character << ":" << e.what() << std::endl;
      throw e;
    }
  }

  //return evaluate(k);
}

Constant
BestfitGroup::evaluate(Context& k)
{
  auto dimtime = k.lookup(DIM_TIME);
  int time = 0;

  if (dimtime.index() == TYPE_INDEX_INTMP)
  {
    time = Types::Intmp::get(dimtime).get_si();
  }

  //first check the last definition
  auto& last = m_evaluators.back();
  if (time >= last.start && (last.end == -1 || time <= last.end))
  {
    return (*last.evaluator)(k);
  }

  //make sure that it's not bigger than the last and smaller than the first
  if (time > last.start || time < m_evaluators.front().start)
  {
    return Types::Special::create(SP_UNDEF);
  }

  //otherwise do a linear search backwards if small, binary search if big
  if (m_evaluators.size() <= LINEAR_SEARCH_MAX_SIZE)
  {
    for (auto i = m_evaluators.rbegin(); i != m_evaluators.rend(); ++i)
    {
      if (time >= i->start && time <= i->end)
      {
        return (*i->evaluator)(k);
      }
    }
    //if we got here then something broke
    return Types::Special::create(SP_UNDEF);
  }
  else
  {
    //binary search the rest
    auto begin = m_evaluators.begin();
    auto end = m_evaluators.end();
    --end;
    auto current = begin + (end - begin) / 2;

    //this is guaranteed to find a match now
    while (!(time >= current->start && time <= current->end))
    {
      if (time < current->start)
      {
        end = current;
      }
      else
      {
        begin = current + 1;
      }
      current = begin + (end - begin) / 2;
    }

    //current is the match
    return (*current->evaluator)(k);
  }
}

template <typename... Delta>
std::pair<bool, std::shared_ptr<Region>>
EquationGuard::evaluate(Context& k, Delta&&... delta) const
{
  if (!m_compiled)
  {
    compile();
  }

  bool nonspecial = true;
  m_demands.clear();
  Region::Entries t = m_dimConstConst;

  if (m_guard)
  {
    //start with the const dimensions and evaluate the non-const ones

    //evaluate the ones left
    for (const auto& constNon : m_dimConstNon)
    {
      Constant ord = constNon.second.second->operator()(k, delta...);

      if (ord.index() == TYPE_INDEX_DEMAND)
      {
        const auto& dims = Types::Demand::get(ord).dims();
        std::copy(dims.begin(), dims.end(), std::back_inserter(m_demands));
      }
      else
      {
        t.insert(std::make_pair(constNon.first,
          std::make_pair(constNon.second.first, ord)));
      }
    }

    for (const auto& nonConst : m_dimNonConst)
    {
      Constant dim = nonConst.first->operator()(k, delta...);

      if (dim.index() == TYPE_INDEX_DEMAND)
      {
        const auto& dims = Types::Demand::get(dim).dims();
        std::copy(dims.begin(), dims.end(), std::back_inserter(m_demands));
      }
      else if (dim.index() == TYPE_INDEX_SPECIAL)
      {
        nonspecial = false;
      }
      else
      {
        dimension_index index =
          dim.index() == TYPE_INDEX_DIMENSION 
          ? get_constant<dimension_index>(dim)
          : m_system->getDimensionIndex(dim);

        t.insert(std::make_pair(index, 
          std::make_pair(nonConst.second.first, nonConst.second.second)));
      }
    }

    for (const auto& nonNon : m_dimNonNon)
    {
      Constant dim = nonNon.first->operator()(k, delta...);
      Constant ord = nonNon.second.second->operator()(k, delta...);

      if (dim.index() == TYPE_INDEX_SPECIAL)
      {
        nonspecial = false;
      }

      bool isdemand = false;

      if (ord.index() == TYPE_INDEX_DEMAND)
      {
        const auto& dims = Types::Demand::get(ord).dims();
        std::copy(dims.begin(), dims.end(), std::back_inserter(m_demands));
        isdemand = true;
      }

      if (dim.index() == TYPE_INDEX_DEMAND)
      {
        const auto& dims = Types::Demand::get(dim).dims();
        std::copy(dims.begin(), dims.end(), std::back_inserter(m_demands));
        isdemand = true;
      }

      if (!isdemand)
      {
        dimension_index index =
          dim.index() == TYPE_INDEX_DIMENSION 
          ? get_constant<dimension_index>(dim)
          : m_system->getDimensionIndex(dim);

        t.insert(std::make_pair(index, 
          std::make_pair(nonNon.second.first, ord)));
      }
    }
  }

  //the tuple doesn't matter here if nonspecial is false, the false
  //says ignore the result
  return std::make_pair(nonspecial, std::make_shared<Region>(t));
}

//how to bestfit with a cache
//  until we find a priority that has valid equations and there are no demands
//  for dimensions, do:
//1. evaluate all the guards that are applicable to the current time
//2. if there are any demands for dimensions then return
//3. evaluate the booleans and guards for applicability
//4. if any of that requires dimensions then return
//5. then bestfit
template <typename... Delta>
Constant
ConditionalBestfitWS::bestfit(const applicable_list& applicable, Context& k, 
  Delta&&... delta)
{

  //for whichever priority something was chosen, they will have been added from
  //oldest to newest, so we now have a list in order of oldest to newest

  //std::cout << "have " << applicable.size() << " applicable equations" 
  //          << std::endl;
  if (applicable.size() == 0)
  {
    //std::cerr << "undef for " << m_name << std::endl;
    return Types::Special::create(SP_UNDEF);
  }
  else if (applicable.size() == 1)
  {
    //std::cerr << "running equation " << std::get<1>(applicable.front())->id()
    //<< std::endl;
    return (*std::get<1>(applicable.front()))(k, delta...);
  }

  //if there is more than applicable equation, find the best
  std::vector<applicable_list::const_iterator> bestIters;

  for (applicable_list::const_iterator i = applicable.begin();
       i != applicable.end(); ++i)
  {
    bool best = true;
    for (applicable_list::const_iterator j = applicable.begin();
         j != applicable.end(); ++j)
    {
      if (i != j && !regionSubset(*std::get<0>(*i), *std::get<0>(*j), true))
      {
        best = false;
      }
    }

    if (best)
    {
      bestIters.push_back(i);
    }
  }

  //the list of best will be in order from oldest to newest, so go through 
  //from the back and find everything with the highest provenance
  if (bestIters.size() >= 1)
  {
    //std::cerr << m_name << ": " << bestIters.size() << " best" << std::endl;
    std::vector<applicable_list::const_iterator> newestBest;

    auto iter = bestIters.rbegin();
    int latest = std::get<1>(**iter)->provenance();
    while (iter != bestIters.rend() &&
           latest == std::get<1>(**iter)->provenance())
    {
      newestBest.push_back(*iter);
      ++iter;
    }

    if (newestBest.size() == 1)
    {
      //std::cerr << m_name << ": One newest" << std::endl;
      return (*std::get<1>(*newestBest.front()))
        (k, delta...);
    }
    else
    {
      //std::cerr << m_name << ": multiple newest" << std::endl;

      return Types::Special::create(SP_MULTIDEF);
      //we can't do bestselect right now because these things aren't named

      #if 0

      //evaluate everything first
      std::vector<Constant> bestEvaluated;
      for (const auto& best : newestBest)
      {
        //best is an applicable_list::const_iterator
        bestEvaluated.push_back((*std::get<1>(*best))
          (k, delta...));
      }

      //find the bestselect
      auto bestselectName = U"bestselect_" + m_name;
      auto findIdent = m_system.lookupIdentifiers();
      WS* bestselect = findIdent.lookup(bestselectName);

      if (bestselect != nullptr)
      {
        Constant fn = (*bestselect)(k);
        return evaluateBestselect(k, bestEvaluated, fn);
      }
      else
      {
        //find default bestselect
        WS* bestselectDefault = findIdent.lookup(U"bestselect__");

        if (bestselectDefault == nullptr)
        {
          //we can blow up here
          throw "No default bestselect defined";
        }

        Constant fn = (*bestselectDefault)(k);
        return evaluateBestselect(k, bestEvaluated, fn);
      }
      #endif
    }
  }
  else
  {
    //std::cerr << m_name << ": no best" << std::endl;
    return Types::Special::create(SP_UNDEF);
  }
 
  //std::cerr << "running equation " << std::get<1>(*bestIter)->id()
  //<< std::endl;

  if (bestIters.size() == 1)
  {
    return (*std::get<1>(*bestIters.front()))(k, delta...);
  }
  else
  {
    //std::cerr << "multidef for " << m_name << std::endl;
    return Types::Special::create(SP_MULTIDEF);
  }
}

Constant
ConditionalBestfitWS::operator()(Context& k)
{

  //std::cerr << "evaluating variable " << m_name 
  //<< ", context: " 
  //<< std::endl;
  //k.print(std::cerr);
  //std::cerr << std::endl;

  applicable_list applicable;
  applicable.reserve(m_equations.size());

  //find all the applicable ones

  //for each priority...
  //if nothing was found at this priority then look at the next one
  for (auto priorityIter = m_priorityVars.rbegin();
       priorityIter != m_priorityVars.rend() && applicable.empty();
       ++priorityIter
  )
  {
    //look at everything created before this time
    const auto& thisPriority = priorityIter->second;
    for (auto provenanceIter = thisPriority.begin();
         provenanceIter != thisPriority.end();
         ++provenanceIter
    )
    {
      const auto& eqn_i = provenanceIter->second;

      if (eqn_i->validContext())
      {
        const EquationGuard& guard = eqn_i->validContext();
        auto result = guard.evaluate(k);

        if (result.first && regionApplicable(*result.second, k)
          && booleanTrue(guard, k)
        )
        {
          applicable.push_back
            (ApplicableTuple(result.second, eqn_i));
        }
      }
      else
      {
        applicable.push_back
          (ApplicableTuple(std::make_shared<Region>(), eqn_i));
      }
    }
  }

  return bestfit(applicable, k);
}

Constant
ConditionalBestfitWS::operator()(Context& kappa, Context& delta)
{
  applicable_list applicable;
  applicable_list potential;
  applicable.reserve(m_equations.size());
  std::vector<dimension_index> demands;

  //find all the applicable ones

  //for each priority...
  //if nothing was found at this priority then look at the next one
  for (auto priorityIter = m_priorityVars.rbegin();
       priorityIter != m_priorityVars.rend() && applicable.empty();
       ++priorityIter
  )
  {
    //make sure there are no potentials from the previous priority
    potential.clear();

    //look at everything created before this time
    const auto& thisPriority = priorityIter->second;
    for (auto provenanceIter = thisPriority.begin();
         provenanceIter != thisPriority.end();
         ++provenanceIter
    )
    {
      const auto& eqn_i = provenanceIter->second;

      //if it has a non-empty context guard
      if (eqn_i->validContext())
      {
        const EquationGuard& guard = eqn_i->validContext();
        auto result = guard.evaluate(kappa, delta);

        if (result.first)
        {
          std::copy(guard.demands().begin(), guard.demands().end(),
            std::back_inserter(demands));

          potential.push_back(ApplicableTuple(result.second, eqn_i));
        }
      }
      else
      {
        potential.push_back(ApplicableTuple(std::make_shared<Region>(), eqn_i));
      }
    }

    if (demands.size() > 0)
    {
      return Types::Demand::create(demands);
    }

    //go through the potential equations and check their applicability
    for (const auto& p : potential)
    {
      const auto& context = std::get<0>(p);
      if (context->begin() == context->end())
      {
        applicable.push_back(p);
      }
      else
      {
        //check that all the dimensions in context are in delta
        bool hasdemands = false;
        for (const auto& index : *context)
        {
          if (!delta.has_entry(index.first))
          {
            demands.push_back(index.first);
            hasdemands = true;
          }
        }

        //don't bother doing this if there are dimensions not available
        //booleanTrue returns false if there are demands
        if (!hasdemands && regionApplicable(*context, kappa)
          && booleanTrue(std::get<1>(p)->validContext(), kappa, delta,
               demands)
        )
        {
          applicable.push_back(p);
        }
      }
    }

    //stop if looking at the tuples generated demands
    if (demands.size() > 0)
    {
      return Types::Demand::create(demands);
    }
  }

  //now we have something valid and no demands are needed
  return bestfit(applicable, kappa, delta);
}

TimeConstant
ConditionalBestfitWS::operator()
  (Context& kappa, Delta& d, const Thread& w, size_t t)
{
}

EquationGuard::EquationGuard(WS* g, WS* b)
: m_guard(g), m_boolean(b), m_compiled(false), m_onlyConst(false),
  m_system(nullptr), m_priority(0)
{
}

EquationGuard::EquationGuard(const EquationGuard& other)
: m_guard(other.m_guard)
, m_boolean(other.m_boolean)
, m_dimConstConst(other.m_dimConstConst)
, m_dimConstNon(other.m_dimConstNon)
, m_dimNonConst(other.m_dimNonConst)
, m_dimNonNon(other.m_dimNonNon)
, m_compiled(other.m_compiled)
, m_onlyConst(other.m_onlyConst)
, m_system(other.m_system)
, m_priority(other.m_priority)
{
}

void
EquationGuard::compile() const
{
  //everything goes into nonconst right now
  if (m_guard == nullptr)
  {
    m_compiled = true;
    return;
  }

  Context k;

  //set time to zero for now
  k.perturb(DIM_TIME, Types::Intmp::create(0));

  //some trickery to evaluate guards at compile time
  Workshops::RegionWS* t = dynamic_cast<Workshops::RegionWS*>(m_guard.get());

  //For now guards must be a literal tuple
  if (t != nullptr)
  {
    const auto& entries = t->getEntries();
    System& s = t->getSystem();
    m_system = &s;

    for (const auto& val : entries)
    {
      bool lhsConst = true;
      bool rhsConst = true;
      Constant lhs = std::get<0>(val)->operator()(k);

      if (lhs.index() == TYPE_INDEX_SPECIAL)
      {
        lhsConst = false;
      }

      dimension_index dimIndex = 
        lhs.index() == TYPE_INDEX_DIMENSION 
        ? get_constant<dimension_index>(lhs)
        : s.getDimensionIndex(lhs);

      Constant rhs = std::get<2>(val)->operator()(k);

      if (rhs.index() == TYPE_INDEX_SPECIAL)
      {
        rhsConst = false;
      }

      //we'll work out whether both left-hand sides and right-hand sides
      //are constant, and stick them in a container to evaluate the
      //remainder later
      if (lhsConst)
      {
        if (rhsConst)
        {
          m_dimConstConst.insert(std::make_pair(dimIndex, 
            std::make_pair(std::get<1>(val), rhs)));
        }
        else
        {
          m_dimConstNon.insert(std::make_pair(dimIndex, 
            std::make_pair(std::get<1>(val), std::get<2>(val))));
        }
      }
      else
      {
        if (rhsConst)
        {
          m_dimNonConst.insert(std::make_pair(std::get<0>(val), 
            std::make_pair(std::get<1>(val), rhs)));
        }
        else
        {
          m_dimNonNon.insert(std::make_pair(std::get<0>(val), 
            std::make_pair(std::get<1>(val), std::get<2>(val))));
        }
      }
      #if 0
      m_dimNonNon.insert(std::make_pair(val.first, val.second));
      #endif
    }

    if (m_dimConstNon.size() == 0 && m_dimNonConst.size() == 0
        && m_dimNonNon.size() == 0)
    {
      m_onlyConst = true;
    }

    //find a priority dimension setting
    auto priorityIter = m_dimConstConst.find(DIM_PRIORITY);
    if (priorityIter != m_dimConstConst.end())
    {
      if (priorityIter->second.second.index() == TYPE_INDEX_INTMP)
      {
        m_priority = Types::Intmp::get(priorityIter->second.second).get_si();
        m_dimConstConst.erase(priorityIter);
      }
    }

    m_compiled = true;
  }
  else
  {
    std::cerr << "guard is not a region" << std::endl;
    throw "guard is not a region";
  }
}

ConditionalBestfitWS::ConditionalBestfitWS(Equations e)
: m_equations(e)
{
  Context emptyk;
  for (auto uiter = m_equations.begin(); uiter != m_equations.end(); ++uiter)
  {
    auto& eqn = *uiter;
    //force the equation to be compiled and get the priority
    eqn.validContext().evaluate(emptyk);

    int time = eqn.provenance();
    
    int priority = eqn.priority();
    
    auto iter = m_priorityVars.find(priority);

    if (iter == m_priorityVars.end())
    {
      iter = m_priorityVars.insert(std::make_pair(priority, ProvenanceList()))
        .first;
    }

    iter->second.push_back(std::make_pair(time, uiter));
  }
  #if 0

  //insert in the priority list
    #endif
}

CompiledEquationWS::CompiledEquationWS
(
  const EquationGuard& valid, 
  std::shared_ptr<WS> evaluator,
  int provenance
)
: m_validContext(valid)
, m_eqn(evaluator)
, m_provenance(provenance)
, m_priority(0)
{
}

//is region a a subset of region b?
bool
regionSubset(const Region& a, const Region& b, bool canequal)
{
  #if 0
  std::cerr << "== tuple refines ==" << std::endl;
  a.print(std::cerr);
  std::cerr << std::endl;
  b.print(std::cerr);
  std::cerr << std::endl;
  #endif
  //for a to refine b, everything in b must be in a, and for the values that 
  //are, they have to be either equal, or their ranges must be more specific
  //but a cannot equal b
  bool equal = true;
  auto it1 = a.begin();
  auto it2 = b.begin();
  while (it1 != a.end() && it2 != b.end())
  {
    type_index d1 = it1->first;
    type_index d2 = it2->first;

    //std::cerr << "tuples have dimensions: " << d1 << " and " << d2 << 
    //  std::endl;

    //extra dimension in b
    if (d2 < d1)
    {
      //std::cerr << "no by extra dimension" << std::endl;
      return false;
    }

    //extra dimension in a
    if (d2 > d1)
    {
      ++it1;
      //a is more specific if the rest passes
      equal = false;
      //std::cerr << "not equal by dimension" << std::endl;
      continue;
    }

    auto spec1 = it1->second.first;
    auto spec2 = it2->second.first;

    //first check if the specifiers are the same
    if (spec1 != spec2)
    {
      //if they are different and they aren't "is" and ":", then we don't have
      //a subset relationship
      if (!(spec1 == Region::Containment::IS && 
              (spec2 == Region::Containment::IN || 
               spec2 == Region::Containment::IMP
              )
          ))
      {
        return false;
      }
      //otherwise a is an "is" and b is a ":", so continue
    }
    else
    {
      //if they are the same, then it depends on the exact values
      if (!valueSmaller(it1->second.second, it1->second.first, 
          it2->second.second))
      {
        //std::cerr << "no by not refines" << std::endl;
        //std::cerr << it1->first << ", " << it2->first << std::endl;
        //std::cerr << it1->second.index() << ", " << it2->second.index() 
        //  << std::endl;
        return false;
      }
      //if they both refine each other they are equal
      else if (!valueSmaller(it2->second.second, it1->second.first,
               it1->second.second))
      {
        //the a value is contained in the b value, so a is more
        //specific as long as the rest passes
        equal = false;
        //std::cerr << "not equal by value" << std::endl;
      }
    }

    ++it1;
    ++it2;
  }

  if (it2 != b.end())
  {
    //b has stuff left, can't refine
    //std::cerr << "no by b iter not at end" << std::endl;
    return false;
  }

  if (it1 != a.end())
  {
    //there is stuff left in a that was never checked
    //therefore it refines
    //std::cerr << "refines" << std::endl;
    return true;
  }

  //if we get here then a is either equal to b or refines it
  //if not equal then the variable equal would have been changed somewhere
  //std::cerr << (!equal ? "yes" : "no") << std::endl;
  return !equal || canequal;
}

//I know that this is almost the exact same code as above, but what can I do?
//Maybe they can be combined into one function
//does a refine b
bool
tupleRefines(const Tuple& a, const Tuple& b, bool canequal)
{
  #if 0
  std::cerr << "== tuple refines ==" << std::endl;
  a.print(std::cerr);
  std::cerr << std::endl;
  b.print(std::cerr);
  std::cerr << std::endl;
  #endif
  //for a to refine b, everything in b must be in a, and for the values that 
  //are, they have to be either equal, or their ranges must be more specific
  //but a cannot equal b
  bool equal = true;
  Tuple::const_iterator it1 = a.begin();
  Tuple::const_iterator it2 = b.begin();
  while (it1 != a.end() && it2 != b.end())
  {
    type_index d1 = it1->first;
    type_index d2 = it2->first;

    //std::cerr << "tuples have dimensions: " << d1 << " and " << d2 << 
    //  std::endl;

    //extra dimension in b
    if (d2 < d1)
    {
      //std::cerr << "no by extra dimension" << std::endl;
      return false;
    }

    //extra dimension in a
    if (d2 > d1)
    {
      ++it1;
      //a is more specific if the rest passes
      equal = false;
      //std::cerr << "not equal by dimension" << std::endl;
      continue;
    }

    if (!valueRefines(it1->second, it2->second))
    {
      //std::cerr << "no by not refines" << std::endl;
      //std::cerr << it1->first << ", " << it2->first << std::endl;
      //std::cerr << it1->second.index() << ", " << it2->second.index() 
      //  << std::endl;
      return false;
    }
    //if they both refine each other they are equal
    else if (!valueRefines(it2->second, it1->second))
    {
      //the a value is contained in the b value, so a is more
      //specific as long as the rest passes
      equal = false;
      //std::cerr << "not equal by value" << std::endl;
    }
    ++it1;
    ++it2;
  }

  if (it2 != b.end())
  {
    //b has stuff left, can't refine
    //std::cerr << "no by b iter not at end" << std::endl;
    return false;
  }

  if (it1 != a.end())
  {
    //there is stuff left in a that was never checked
    //therefore it refines
    //std::cerr << "refines" << std::endl;
    return true;
  }

  //if we get here then a is either equal to b or refines it
  //if not equal then the variable equal would have been changed somewhere
  //std::cerr << (!equal ? "yes" : "no") << std::endl;
  return !equal || canequal;
}
//does value a refine value b
bool
valueRefines(const Constant& a, const Constant& b)
{
  //this is implemented by creating a matrix of function pointers
  //which is as big as the largest type index that we need to consider

  //std::cerr << "== value refines ==" << std::endl;
  //std::cerr << a << " r " << b << std::endl;
  //if b is a range, a has to be a range and within or equal,
  //or an int and inside, otherwise they have to be equal

  IsSubsetOf f = typeCompare.getComparator(b.index());
  if (f != 0)
  {
    IsSubsetFn sub = f(a);
    return sub(a, b);
  }
  else
  {
    //there is no comparator, just check if they are equal
    return a == b;
  }
}

bool
valueInside
(
  const Constant& smaller, 
  Region::Containment specifier, 
  const Constant& bigger
)
{
  switch (specifier)
  {
    case Region::Containment::IN:
    //here we check if smaller is a subset of bigger
    return valueRefines(smaller, bigger);
    break;

    case Region::Containment::IS:
    return smaller == bigger;
    break;

    case Region::Containment::IMP:
    if (bigger.index() == TYPE_INDEX_TYPE)
    {
      return smaller.index() == get_constant<type_index>(bigger);
    }
    return false;
    break;
  }

  //this is to satisfy the compiler that we have a return value
  return false;
}

bool
valueSmaller
(
  const Constant& smaller, 
  Region::Containment specifier, 
  const Constant& bigger
)
{
  switch (specifier)
  {
    case Region::Containment::IN:
    //here we check if smaller is a subset of bigger
    return valueRefines(smaller, bigger);
    break;

    case Region::Containment::IS:
    return smaller == bigger;
    break;

    case Region::Containment::IMP:
    return smaller == bigger;
    break;
  }

  //this is to satisfy the compiler that we have a return value
  return false;
}

template <typename T>
bool
regionApplicable(const Region& r, const T& t)
{
  for (const auto& set : r) 
  {
    Constant val = TupleLookup<T>()(t, set.first);
    if (!valueInside(val, set.second.first, set.second.second))
    {
      return false;
    }
  }

  //if def has nothing it is applicable
  return true;
}

} //namespace TransLucid
