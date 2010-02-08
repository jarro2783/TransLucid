#include <tl/utility.hpp>
#include <tl/equation.hpp>
#include <tl/types.hpp>

namespace TransLucid
{

const u32string
SplitID::m_split_match = U".";

bool
tupleApplicable(const Tuple& def, const Tuple& c)
{
  //all of def has to be in c, and the values have to either be
  //equal or within the range
  for (Tuple::const_iterator iter = def.begin(); iter != def.end(); ++iter)
  {
    Tuple::const_iterator citer = c.find(iter->first);
    if (citer == c.end())
    {
      return false;
    }
    else
    {
      if (!valueRefines(citer->second, iter->second))
      {
        return false;
      }
    }
  }

  //if def has nothing it is applicable
  return true;
}


//does value a refine value b
bool
valueRefines(const TypedValue& a, const TypedValue& b)
{
  //if b is a range, a has to be a range and within or equal,
  //or an int and inside, otherwise they have to be equal

  //three cases, range, type, anything else

  if (b.index() == TYPE_INDEX_RANGE)
  {
    if (a.index() == TYPE_INDEX_RANGE)
    {
      if (!b.value<Range>().within(a.value<Range>()))
      {
        return false;
      }
    }
    else if (a.index() == TYPE_INDEX_INTMP)
    {
      if (!b.value<Range>().within(a.value<Intmp>()))
      {
        return false;
      }
    }
    else
    {
      return false;
    }
    return true;
  }
  else if (b.index() == TYPE_INDEX_TYPE)
  {
    if (a.index() == b.value<TypeType>().index())
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return a == b;
  }

  //for now a and b just have to be equal
  return a == b;
}

//does a refine b
bool
tupleRefines(const Tuple& a, const Tuple& b)
{
  //for a to refine b, everything in b must be in a, and for the values that are,
  //they have to be either equal, or their ranges must be more specific
  Tuple::const_iterator it1 = a.begin();
  Tuple::const_iterator it2 = b.begin();
  while (it1 != a.end() && it2 != b.end())
  {
    type_index d1 = it1->first;
    type_index d2 = it2->first;

    //extra dimension in b
    if (d2 < d1)
    {
      return false;
    }

    //extra dimension in a
    if (d1 > d2)
    {
      ++it1;
      continue;
    }

    if (!valueRefines(it1->second, it2->second))
    {
      return false;
    }
    ++it1;
    ++it2;
  }

  if (it2 != b.end())
  {
    return false;
  }
  return true;
}

bool
booleanTrue(const EquationGuard& g, const Tuple& k)
{
  HD* b = g.boolean();

  if (b)
  {
    TaggedValue v = (*b)(k);// = i.evaluate(g.boolean(), c);

    return v.first.index() == TYPE_INDEX_BOOL
    && v.first.value<Boolean>();
  }
  else
  {
    return true;
  }
}

}
