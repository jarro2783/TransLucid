/* Trie implementation.
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
 * @file trie.hpp
 * Trie implementation.
 */

//NOTE: this has been put on the sideline until UUID deletion and replacement
//works by inputting the full UUID. Currently the only use of this trie will
//be to input partial UUIDs.

#ifndef TL_TRIE_HPP_INCLUDED
#define TL_TRIE_HPP_INCLUDED

#include <map>

namespace TransLucid
{
  //a trie with a map for each level
  //which maps arrays of Unit to Rhs
  template <typename Unit, typename Rhs>
  class MapTrie
  {
    public:

    typedef std::shared_ptr<Rhs> RhsPtr;

    enum FindResult
    {
      OK,
      AMBIGUOUS,
      NOT_FOUND
    };
    
    //insert the pair (key, rhs) where the key array has length keyLength
    void
    insert(Unit* key, int keyLength, Rhs rhs)
    {
    }

    std::pair<FindResult, RhsPtr>
    find(Unit* key, int length)
    {
    }

    private:

    struct Level
    {
      typedef std::map<Unit, std::pair<RhsPtr, Level>> LevelNode;
      LevelNode node;
    };

    Level m_top;
  };
}

#endif
