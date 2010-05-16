/* TransLucid exceptions.
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

#ifndef EXCEPTION_HPP_INCLUDED
#define EXCEPTION_HPP_INCLUDED

#include <tl/types.hpp>
#include <tl/charset.hpp>

namespace TransLucid
{

  /**
   * @brief Internal exceptions.
   *
   * An enum describing the possible internal exceptions that
   * can be thrown by the system.
   **/
  enum ExceptionType
  {
    EXCEPTION_INTERNAL,
    EXCEPTION_PARSE
  };

  class Exception : public std::exception
  {
  };

  template <ExceptionType T>
  class TemplateException : public Exception
  {
    public:
    TemplateException(const u32string& m)
    : message(utf32_to_utf8(m))
    {
    }

    ~TemplateException() throw()
   {}

    const char*
    what() const throw()
    {
      return message.c_str();
    }

    private:
    std::string message;
  };

  /**
   * @brief Internal Error exception.
   *
   * Exception which is thrown when the system detects that it
   * is broken.
   **/
  typedef TemplateException<EXCEPTION_INTERNAL> InternalError;

  /**
   * @brief Parse Error exception.
   *
   * Exception that is thrown when a parse error occurs.
   **/
  typedef TemplateException<EXCEPTION_PARSE> ParseError;
}

#endif // EXCEPTION_HPP_INCLUDED
