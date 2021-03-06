/* Base function workshop code
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
 * @file function.hpp
 * Function definition workshop.
 */

#ifndef TL_BASEFUN_HPP_INCLUDED
#define TL_BASEFUN_HPP_INCLUDED

#include <tl/bestfit.hpp>
#include <tl/workshop.hpp>

namespace TransLucid
{
  class BaseFunctionWS : public WS, public DefinitionGrouper
  {
    private:

    BestfitGroup m_bestfit;
  };
}

#endif
