/* Collapses where clauses
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
 * @file collapse.hpp
 * Collapses where clauses so that there are no wherevar clauses appearing
 * in the expressions. The where clauses will still be in the tree but
 * effectively only as wheredim clauses. The output is all the functions 
 * and variables that were taken out of where clauses.
 */

#include <tl/generic_walker.hpp>

namespace TransLucid
{
  class CollapseWhere : public GenericTreeWalker<CollapseWhere>
  {
    public:

    typedef Tree::Expr result_type;
  };
}
