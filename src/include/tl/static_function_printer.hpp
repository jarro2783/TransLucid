/* Static function printer
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

#ifndef TL_STATIC_FUNCTION_PRINTER_HPP
#define TL_STATIC_FUNCTION_PRINTER_HPP

#include <tl/static_function.hpp>
#include <tl/output.hpp>

namespace TransLucid
{
  //this has to be here, because Static::Functor is actually a 
  //TransLucid::Variant of stuff
  template <typename Prop>
  std::ostream& operator<<(std::ostream& os,
    const Static::Functor<Prop>& f)
  {
    os << Static::print_functor(f);
    return os;
  }

  namespace Static
  {
    namespace detail
    {
      template <typename Prop>
      class FunctorPrinter
      {
        public:

        typedef u32string result_type;

        u32string
        operator()(const Functions::Topfun& top) const
        {
          return U"⊤";
        }

        u32string
        operator()(const Functions::Param& p) const
        {
          ostringstream32 os;
          os << "(param " << p.dim << ")";
          //return u32string(os.str().begin(), os.str().end());
          return os.str();
        }

        u32string
        operator()(const Functions::ApplyV<Prop>& applyv) const
        {
          std::ostringstream os;

          os << "(applyv ";

          os << apply_visitor(*this, applyv.lhs);

          os << " {";

          print_container(os, applyv.rhs);

          os << "})";
          
          auto str = os.str();
          return u32string(str.begin(), str.end());
        }

        u32string
        operator()(const Functions::ApplyB<Prop>& applyb) const
        {
          std::ostringstream os;

          os << "(applyb ";

          os << apply_visitor(*this, applyb.lhs);

          for (const auto& param : applyb.params)
          {
            os << " {";

            print_container(os, param);

            os << "}";
          }

          os << ")";
          
          auto str = os.str();
          return u32string(str.begin(), str.end());
        }

        u32string
        operator()(const Functions::Down<Prop>& down) const
        {
          std::ostringstream os;
          os << "(down ";
          os << apply_visitor(*this, down.body);
          os << ")";

          auto str = os.str();
          return u32string(str.begin(), str.end());
        }

        u32string
        operator()(const Functions::Up<Prop>& up) const
        {
          std::ostringstream os;
          os << "(up {";

          std::copy(up.property.begin(), up.property.end(),
            std::ostream_iterator<typename Prop::value_type>(os, ", "));
          //print_container(os, up.property);

          os << "} {";

          print_container(os, up.functions);
          //std::copy(up.property.begin(), up.property.end(),
          //  std::ostream_iterator<Functor<Prop>>(os, ", "));

          os << "})";
          
          auto str = os.str();

          return u32string(str.begin(), str.end());
        }

        u32string
        operator()(const Functions::CBV<Prop>& cbv) const
        {
          std::ostringstream os;
          os << "(cbv " << cbv.dim << " {";

          std::copy(cbv.property.begin(), cbv.property.end(),
            std::ostream_iterator<typename Prop::value_type>(os, ", "));

          os << "} {";

          print_container(os, cbv.functions);

          os << "})";

          auto str = os.str();

          return u32string(str.begin(), str.end());
        }

        u32string
        operator()(const Functions::Base<Prop>& base) const
        {
          std::ostringstream os;
          os << "(base (";
          std::copy(base.dims.begin(), base.dims.end(),
            std::ostream_iterator<typename decltype(base.dims)::value_type>
              (os, ", "));

          os << ") {";

          std::copy(base.property.begin(), base.property.end(),
            std::ostream_iterator<typename Prop::value_type>(os, ", "));

          os << "} {";

          print_container(os, base.functions);

          os << "})";

          auto str = os.str();

          return u32string(str.begin(), str.end());
        }
      };
    }

    template <typename Prop>
    u32string
    print_functor(const Functor<Prop>& f)
    {
      //std::cout << f;
      return apply_visitor(detail::FunctorPrinter<Prop>(), f);
    }
  }
}

#endif
