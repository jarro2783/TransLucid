/* The TransLucid container.
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

    uuid
    addExpr(const Tuple& k, HD* e);

    TaggedConstant
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
