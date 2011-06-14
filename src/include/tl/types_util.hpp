/* Types utility functions.
   Copyright (C) 2011 Jarryd Beck and John Plaice

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

#ifndef TL_TYPES_UTIL_HPP_INCLUDED
#define TL_TYPES_UTIL_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{
  template <typename T>
  void
  delete_ptr(void* p)
  {
    delete reinterpret_cast<T*>(p);
  }

  template <typename T>
  Constant
  make_constant_pointer(const T& v, TypeFunctions* funs, type_index index)
  {
    std::unique_ptr<T> value(detail::clone<T>()(v));
    ConstantPointerValue* p = 
      new ConstantPointerValue(funs, value.get());
    value.release();
    return Constant(p, index);
  }

  template <typename T>
  const T&
  get_constant_pointer(const Constant& c)
  {
    return *static_cast<T*>(c.data.ptr->data);
  }
}

#endif
