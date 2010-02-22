#include <tl/equation.hpp>
#include <tl/range.hpp>
#include <tl/utility.hpp>

namespace TransLucid
{

Tuple
EquationGuard::evaluate(const Tuple& k) const
throw (InvalidGuard)
{
  tuple_t t = m_dimensions;

  if (m_guard)
  {
    TaggedValue v = (*m_guard)(k);
    //ValueContext v = i.evaluate(m_guard, context);

    //still need to remove this magic
    if (v.first.index() == TYPE_INDEX_TUPLE)
    {
      BOOST_FOREACH(const Tuple::value_type& value, v.first.value<Tuple>())
      {
        if (t.find(value.first) != t.end())
        {
          throw InvalidGuard();
        }

        t.insert(std::make_pair(value.first, value.second));
      }
    }
    else
    {
      throw ParseError(__FILE__ ":" STRING_(__LINE__)
                       U": guard is not a tuple");
    }
  }

  return Tuple(t);
  //TaggedValue v = (*m_guard)(k);
}

inline void
Variable::addExprActual(const Tuple& k, HD* h)
{
  const EquationGuard* g = 0;
  Tuple::const_iterator giter = k.find(DIM_VALID_GUARD);
  if (giter != k.end())
  {
    g = &giter->second.value<EquationGuardType const&>().value();
  }

  if (g)
  {
    m_equations.push_back(Equation(m_name, *g, h));
  }
  else
  {
    m_equations.push_back(Equation(m_name, EquationGuard(), h));
  }
}

TaggedValue
Variable::operator()(const Tuple& k)
{

  //std::cout << "evaluating variable "
  //          << m_name << ", context: " << std::endl;
  //c.print(i, std::cout, c);

  typedef std::tuple<Tuple, HD*> ApplicableTuple;
  typedef std::list<ApplicableTuple> applicable_list;
  applicable_list applicable;

  //find all the applicable ones

  Tuple::const_iterator iditer = k.find(DIM_ID);

  if (iditer != k.end())
  {
    try
    {
      VariableMap::const_iterator viter =
        m_variables.find(iditer->second.value<String>().value());
      //std::cout << "looking for "
      //          << iditer->second.value<String>().value() << std::endl;
      if (viter == m_variables.end())
      {
        //std::cout << "not found" << std::endl;
        return TaggedValue(TypedValue(Special(Special::UNDEF),
                           TYPE_INDEX_SPECIAL), k);
      }
      else
      {
        tuple_t kp = k.tuple();
        kp.erase(DIM_ID);
        return (*viter->second)(Tuple(kp));
      }
    }
    catch (std::bad_cast& e)
    {
      return TaggedValue(TypedValue(Special(Special::DIMENSION),
                         TYPE_INDEX_SPECIAL), k);
    }
  }

  for (Equations::const_iterator eqn_i = m_equations.begin();
      eqn_i != m_equations.end(); ++eqn_i)
  {
    if (eqn_i->validContext())
    {
      try
      {
        const EquationGuard& guard = eqn_i->validContext();
        Tuple evalContext = guard.evaluate(k);
        if (tupleApplicable(evalContext, k) && booleanTrue(guard, k))
        {
          applicable.push_back
            (ApplicableTuple(evalContext, eqn_i->equation()));
         }
      }
      catch (InvalidGuard& e)
      {
      }
    }
    else
    {
      applicable.push_back
        (ApplicableTuple(Tuple(), eqn_i->equation()));
    }
  }

  if (applicable.size() == 0)
  {
    return TaggedValue(TypedValue(Special(Special::UNDEF),
                       TYPE_INDEX_SPECIAL),k);
  }
  else if (applicable.size() == 1)
  {
    return (*std::get<1>(applicable.front()))(k);
  }

  applicable_list::const_iterator bestIter = applicable.end();

  for (applicable_list::const_iterator iter = applicable.begin();
       iter != applicable.end(); ++iter)
  {
    if (bestIter == applicable.end())
    {
      bestIter = iter;
    }
    else if (tupleRefines(std::get<0>(*iter), std::get<0>(*bestIter)))
    {
      bestIter = iter;
    }
    else if (!tupleRefines(std::get<0>(*bestIter), std::get<0>(*iter)))
    {
      bestIter = applicable.end();
    }
  }

  if (bestIter == applicable.end())
  {
    return TaggedValue(TypedValue(Special(Special::UNDEF),
                       TYPE_INDEX_SPECIAL), k);
  }

  for (applicable_list::const_iterator iter = applicable.begin();
       iter != applicable.end(); ++iter)
  {
    if (std::get<1>(*bestIter) != std::get<1>(*iter) &&
        !tupleRefines(std::get<0>(*bestIter), std::get<0>(*iter)))
    {
      return TaggedValue(TypedValue(Special(Special::MULTIDEF),
                         TYPE_INDEX_SPECIAL), k);
    }
  }

  return (*std::get<1>(*bestIter))(k);
}

void
Variable::addExpr(const Tuple& k, HD* e)
{
  size_t dim_id = DIM_ID;
  Tuple::const_iterator iter = k.find(dim_id);
  if (iter == k.end())
  {
    addExprActual(k, e);
  }
  else
  {
    const String* id = iter->second.valuep<String>();
    if (id == 0)
    {
      return;
    }

    SplitID split(id->value());

    //add the equation, don't add any id dimension if the end is empty
    u32string begin = split.first();
    u32string end = split.last();

    tuple_t kp = k.tuple();
    if (end.size() != 0)
    {
      kp[dim_id] = TypedValue(String(end), TYPE_INDEX_USTRING);
    }
    else
    {
      kp.erase(dim_id);
    }

    #if 0
    VariableMap::iterator viter = m_variables.find(begin);
    if (viter == m_variables.end())
    {
      viter = m_variables.insert
                (std::make_pair(begin, new Variable(begin, m_i))).first;
    }
    viter->second->addExpr(Tuple(kp), e);
    #endif
    addToVariableActual(begin, Tuple(kp), e);
  }
}

void
Variable::addToVariableActual(const u32string& id, const Tuple& k, HD* h)
{
  //std::cerr << "addToVariableActual: " <<
  //   id << std::endl;
  //find the variable
  VariableMap::const_iterator iter = m_variables.find(id);
  if (iter == m_variables.end())
  {
    //std::cerr << "constructing new variable" << std::endl;
    iter = m_variables.insert(std::make_pair
                              (id, new Variable(id, m_system))).first;
  }
  iter->second->addExpr(k, h);
}

}
