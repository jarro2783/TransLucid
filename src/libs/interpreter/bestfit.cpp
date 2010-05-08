#include <tl/bestfit.hpp>

namespace TransLucid
{

//TODO finish this
TaggedValue CompileBestFit::operator()(Tuple& k)
{
  BestFit* b = 0;
  BestFit* old = m_bestFittable->setBestFit(b);
  if (old != this) {
  }

  return (*b)(k);
}

} //namespace TransLucid
