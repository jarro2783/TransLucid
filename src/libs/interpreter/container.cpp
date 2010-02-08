#include <tl/container.hpp>

//-1 represents infinity here

namespace TransLucid
{

void
GeneralContainer::addExpr(const Tuple& k, HD* e)
{
}

TaggedValue
GeneralContainer::operator()(const Tuple& k)
{
}

void
GeneralContainer::push_back(HD* v, int time)
{
  mpq_class next = m_storage.size() == 0
                 ? mpq_class(0)
                 : m_storage.rbegin()->first + 1;

  TimedValues entry;
  entry.push_back(SingleValue(time, -1, v));

  m_storage.insert(m_storage.end(), std::make_pair(next, entry));
}

void
GeneralContainer::push_front(HD* v, int time)
{
  mpq_class previous = m_storage.size() == 0
                     ? mpq_class(0)
                     : m_storage.begin()->first - 1;

  TimedValues entry;
  entry.push_back(SingleValue(time, -1, v));

  m_storage.insert(m_storage.begin(), std::make_pair(previous, entry));
}

void
GeneralContainer::replace_at(HD* v, const mpq_class& pos, int time)
{
  Storage::iterator iter = m_storage.lower_bound(pos);

  if (iter != m_storage.end() && iter->first == pos)
  {
    TimedValues& previous = iter->second;
    previous.back().get<1>() = time;
    previous.push_back(SingleValue(time, -1, v));
  }
  else
  {
    TimedValues entry;
    entry.push_back(SingleValue(time, -1, v));
    m_storage.insert(iter, std::make_pair(pos, entry));
  }
}

void
GeneralContainer::insert_before(HD* v, const mpq_class& pos, int time)
{

  #if 0
  Storage::iterator iter = m_storage.begin();
  TimedValues entry;
  entry.push_back(SingleValue(time, -1, v));

  while (iter != m_storage.end() && pos < iter->first)
  {
    ++iter;
  }

  if (iter == m_storage.begin())
  {
    m_storage.insert(std::make_pair(pos-1, entry));
  }
  else
  {
    Storage::iterator previous = iter;
    --previous;
    m_storage.insert(std::make_pair((pos+previous->first)/2, entry));
  }
  #endif

  Storage::iterator iter = m_storage.lower_bound(pos);

  TimedValues entry;
  entry.push_back(SingleValue(time, -1, v));

  if (iter == m_storage.begin())
  {
    m_storage.insert(iter, std::make_pair(pos-1, entry));
  }
  else
  {
    Storage::iterator previous = iter;
    --previous;
    m_storage.insert(iter, std::make_pair((pos+previous->first)/2, entry));
  }

}

void
GeneralContainer::insert_after(HD* v, const mpq_class& pos, int time)
{
  Storage::iterator iter = m_storage.upper_bound(pos);

  TimedValues entry;
  entry.push_back(SingleValue(time, -1, v));

  if (iter == m_storage.end())
  {
    m_storage.insert(iter, std::make_pair(pos + 1, entry));
  }
  else
  {
    m_storage.insert(iter, std::make_pair((pos+iter->first)/2, entry));
  }
}

void
GeneralContainer::remove
(
  const mpq_class& start,
  const mpq_class& end,
  int time
)
{
  Storage::iterator iter = m_storage.lower_bound(time);

  while (iter != m_storage.end() && iter->first < end)
  {
    SingleValue& v = iter->second.back();
    if (v.get<1>() == -1)
    {
      v.get<1>() = time;
    }
    ++iter;
  }
}

}
