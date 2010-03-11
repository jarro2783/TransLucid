#ifndef PRINTER_HPP_INCLUDED
#define PRINTER_HPP_INCLUDED

#include <tl/hyperdaton.hpp>

namespace TransLucid
{
  class ValuePrinter : public HD
  {
    public:

    TaggedValue
    operator()(const Tuple& k);
  };
}

#endif // PRINTER_HPP_INCLUDED
