#ifndef EXCEPTION_HPP_INCLUDED
#define EXCEPTION_HPP_INCLUDED

#include <tl/types.hpp>

namespace TransLucid
{

  /**
   * @brief Internal exceptions.
   *
   * An enum describing the possible internal exceptions that
   * can be thrown by the interpreter.
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
    : m(m)
    {
    }

    ~TemplateException() throw()
   {}

    const char*
    what() const throw()
    {
      return "don't know how to convert u32string to char*";
    }

    private:
    u32string m;
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
