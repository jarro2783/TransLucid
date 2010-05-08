#include <tl/types.hpp>

namespace TransLucid 
{
  class BestFit
  {
    public:
    virtual ~BestFit() {}
    virtual TaggedValue operator()(const Tuple& k) = 0;
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

    TaggedValue operator()(Tuple& k)
    {
      return (*m_bestFit)(k);
    }

    private:
    BestFit* m_bestFit;
  };

  class CompileBestFit : public BestFit
  {
    public:
    TaggedValue operator()(Tuple& k);

    private:
    ~CompileBestFit() {}

    BestFittable* m_bestFittable;
  };

  class BruteForceBestFit : public BestFit
  {
    public:
    TaggedValue operator()(Tuple& k);
  };

  class SingleDefinitionBestFit : public BestFit
  {
  };
}
