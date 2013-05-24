/* Static function template
   Copyright (C) 2013 Jarryd Beck

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

#include <tl/variant.hpp>
#include <tl/types.hpp>

#include <list>
#include <set>

namespace TransLucid
{
  namespace Static
  {
    namespace Functions
    {
      struct Param;

      template <typename Prop>
      struct ApplyV;

      template <typename Prop>
      struct ApplyB;

      template <typename Prop>
      struct Down;

      template <typename Prop>
      struct Up;

      template <typename Prop>
      struct CBV;

      template <typename Prop>
      struct Base;
    }

    template <typename Prop>
    using Functor = Variant
      <
        Functions::Param,
        recursive_wrapper<Functions::ApplyV<Prop>>,
        recursive_wrapper<Functions::ApplyB<Prop>>,
        recursive_wrapper<Functions::Down<Prop>>,
        recursive_wrapper<Functions::Up<Prop>>,
        recursive_wrapper<Functions::CBV<Prop>>,
        recursive_wrapper<Functions::Base<Prop>>
      >;

    namespace Functions
    {

      struct Param
      {
        dimension_index dim;
      };

      template <typename Prop>
      struct CBV
      {
        dimension_index dim;
        Prop property;
        std::list<Functor<Prop>> functions;
      };

      template <typename Prop>
      struct Base
      {
        std::list<dimension_index> dims;
        Prop property;
        std::list<Functor<Prop>> functions;
      };

      template <typename Prop>
      struct Up
      {
        Prop property;
        std::list<Functor<Prop>> functions;
      };

      template <typename Prop>
      struct ApplyV
      {
        Functor<Prop> lhs;
        std::list<Functor<Prop>> rhs;
      };

      template <typename Prop>
      struct ApplyB
      {
      };

      template <typename Prop>
      struct Down
      {
        Functor<Prop> body;
      };

    }
  }
}
