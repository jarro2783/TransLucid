#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED

#include <tl/hyperdaton.hpp>
#include <map>
#include <boost/tuple/tuple.hpp>
#include <gmpxx.h>

namespace TransLucid
{
  class GeneralContainer : public HD
  {
    public:

    void
    addExpr(const Tuple& k, HD* e);

    TaggedValue
    operator()(const Tuple& k);

    private:

    void
    push_back(HD* v, int time);

    void
    push_front(HD* v, int time);

    void
    replace_at(HD* v, const mpq_class& pos, int time);

    void
    insert_before(HD* v, const mpq_class& pos, int time);

    void
    insert_after(HD* v, const mpq_class& pos, int time);

    void
    remove(const mpq_class& start, const mpq_class& end, int time);

    typedef boost::tuple<int, int, HD*> SingleValue;
    typedef std::vector<SingleValue> TimedValues;
    typedef std::map<mpq_class, TimedValues> Storage;

    Storage m_storage;
  };
}

#endif // CONTAINER_HPP_INCLUDED
