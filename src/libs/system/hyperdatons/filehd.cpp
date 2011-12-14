/* FileIO hyperdaton functions.
   Copyright (C) 2011 Jarryd Beck.

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
 * @file filehd.cpp
 * File hyperdaton implementation.
 */

#include <fstream>
#include <algorithm>

#include <tl/hyperdatons/filehd.hpp>
#include <tl/output.hpp>
#include <tl/parser.hpp>
#include <tl/parser_iterator.hpp>
#include <tl/system.hpp>
#include <tl/types_basic.hpp>
#include <tl/types/hyperdatons.hpp>
#include <tl/types/dimension.hpp>
#include <tl/types/intmp.hpp>
#include <tl/types/string.hpp>
#include <tl/utility.hpp>

#include "tl/lexertl.hpp"
#include "tl/lexer_tokens.hpp"

namespace TransLucid
{

namespace TL = TransLucid;

namespace
{

struct array_initialiser;

//this is a hack, for some reason ArrayInit can't have Expr directly in it
//because constructing with array_initialiser is ambiguous
//it sounds like a bug
struct TreeHolder
{
  Tree::Expr expr;
};

typedef Variant<TreeHolder, recursive_wrapper<array_initialiser>> ArrayInit;

struct array_initialiser
{
  std::vector<ArrayInit> array;

  const decltype(array)*
  operator->() const
  {
    return &array;
  }

  decltype(array)*
  operator->()
  {
    return &array;
  }
};

ArrayInit
parse_array_init(
  System& s,
  Parser::LexerIterator& begin,
  const Parser::LexerIterator& end,
  size_t maxNum, 
  size_t current
)
{
  //parses comma separated list of stuff
  //if maxNum == current then the stuff is strings
  //otherwise it is array initialisers

  if (*begin != Parser::TOKEN_LBRACE)
  {
    throw "expected {";
  }

  ++begin;

  array_initialiser a;

  while (true)
  {
    if (maxNum == current)
    {
      #if 0
      if (*begin != Parser::TOKEN_CONSTANT)
      {
        throw "expected constant in array initialiser";
      }

      const auto& value = 
        get<std::pair<u32string, u32string>>(begin->getValue());

      if (value.first != U"ustring")
      {
        throw "expected string constant in array initialiser";
      }
      #endif

      Tree::Expr expr;
      if (s.parseExpression(begin, end, expr))
      {
        a->push_back(TreeHolder{expr});
      }
      else
      {
        throw "Expected expression in initialiser";
      }
    }
    else
    {
      a->push_back(parse_array_init(s, begin, end, maxNum, current + 1));
    }
    
    if (*begin != Parser::TOKEN_COMMA)
    {
      break;
    }
    else
    {
      ++begin;
    }
  }

  if (*begin != Parser::TOKEN_RBRACE)
  {
    throw "expected }";
  }

  ++begin;

  return ArrayInit(a);
}

void
count_dims
(
  std::vector<size_t>& lengths, 
  const ArrayInit& data, 
  size_t current
)
{
  auto p = get<array_initialiser>(&data);

  if (p)
  {
    auto l = (*p)->size();
    if (l > lengths.at(current))
    {
      lengths.at(current) = l;

      for (auto sub : p->array)
      {
        count_dims(lengths, sub, current + 1);
      }
    }
  }
}

struct fill_array
{
  const std::vector<size_t>& m_max;
  System& m_s;

  fill_array
  (
    const std::vector<size_t>& max,
    System& s
  )
  : m_max(max), m_s(s)
  {
  }

  Constant*
  do_fill
  (
    Constant* data,
    const array_initialiser& entry,
    size_t depth
  )
  {
    
    //the only entries that should have constants in them should be
    //when we are at depth == max.size() - 1

    Constant* nextspot = data;
    if (depth == m_max.size() - 1)
    {
      for (size_t current = 0; current != entry->size(); ++current)
      {
        const TreeHolder& nextentry = 
          get<TreeHolder>(entry->at(current));

        //construct the Constant here and put it in the array
        *nextspot = m_s.evalExpr(nextentry.expr);
        ++nextspot;
      }

      auto firstzero = nextspot;

      //compute the address of the next row
      nextspot += m_max.at(depth) - (nextspot - data);

      //TODO fill in zeros
      while (firstzero != nextspot)
      {
        *firstzero = Types::Special::create(SP_CONST);
        ++firstzero;
      }
    }
    else
    { 
      size_t current;
      for (current = 0; current != entry->size(); ++current)
      {
        const array_initialiser& nextentry = 
          get<array_initialiser>(entry->at(current));

        nextspot = do_fill(nextspot, nextentry, depth+1);
      }

      if (entry->size() != m_max.at(depth))
      {
        //find the next entry
        auto maxindex = m_max.begin() + depth;
        ++maxindex;

        size_t increment = std::accumulate(maxindex, m_max.end(), 1, 
          std::multiplies<size_t>());

        nextspot += (m_max.at(depth) - current) * increment;
      }
    }

    return nextspot;
  }

  //data must only have dims number of dimensions of data in it
  void
  operator()
    (Constant* dest, const ArrayInit& data, size_t dims)
  {
    //now fill it in

    //array of the current index
    std::vector<size_t> index(m_max.size(), 0);

    //the first set of entries
    const auto& first = get<array_initialiser>(data);

    do_fill(dest, first, 0);
  }
};

Constant
get_constructor(const u32string& type, System& s, Context& k)
{
  u32string construct = U"construct_" + type;

  WS* fn = s.lookupIdentifiers().lookup(construct);

  if (fn == nullptr)
  {
    throw "Could not find constructor for type";
  }
  else
  {
    Constant constructor = (*fn)(k);

    if (constructor.index() != TYPE_INDEX_BASE_FUNCTION)
    {
      throw "Invalid type for constructor";
    }

    return constructor;
  }
}

template <typename Iterator>
const Constant*
printEntries
(
  std::ostream& os,
  Iterator begin,
  Iterator end,
  const Constant* data,
  WS* print,
  Context& k
)
{
  if (begin == end)
  {
    ContextPerturber p(k, {{DIM_ARG0, *data}});
    Constant result = print->operator()(k);

    os << Types::String::get(result);

    return ++data;
  }
  else
  {
    os << "{";

    const Constant* next = data;
    size_t i = 0;

    auto nextiter = begin;
    ++nextiter;

    next = printEntries(os, nextiter, end, next, print, k);
    ++i;

    while (i != begin->second)
    {
      os << ", ";
      next = printEntries(os, nextiter, end, data, print, k);
      ++i;
    }

    os << "}";
    return next;
  }
}

}

FileArrayInHD::FileArrayInHD(const u32string& file, System& s)
: InputHD(0)
, m_data(nullptr)
{
  std::ifstream in(utf32_to_utf8(file).c_str());

  if (!in.is_open())
  {
    throw "Could not open file";
  }

  //first read in the whole file
  std::string content = read_file(in);

  Parser::StreamPosIterator rawbegin(
    Parser::makeUTF8Iterator(content.begin()));
  Parser::StreamPosIterator rawend(Parser::makeUTF8Iterator(content.end()));

  //something is rotten in the state of denmark
  Context& k = s.getDefaultContext();

  auto idents = s.lookupIdentifiers();

  Parser::LexerIterator lexer(rawbegin, rawend, k, idents);
  auto end = lexer.makeEnd();

  Parser::Parser parser(s);

  if (*lexer != Parser::TOKEN_ID || 
      TransLucid::get<u32string>(lexer->getValue()) != U"indexedby")
  {
    throw "Expected identifier \"indexedby\"";
  }

  ++lexer;

  if (*lexer != Parser::TOKEN_LBRACE)
  {
    throw "Expected {";
  }

  ++lexer;

  //parse a comma separated list of exprs
  std::vector<Tree::Expr> index;
  while (true)
  {
    Tree::Expr dim;
    if (!parser.parse_expr(lexer, end, dim))
    {
      throw "Expected expression";
    }

    index.push_back(dim);

    if (*lexer != Parser::TOKEN_COMMA)
    {
      break;
    }
    ++lexer;
  }

  if (*lexer != Parser::TOKEN_RBRACE)
  {
    throw "Expected }";
  }

  ++lexer;

  if (*lexer != Parser::TOKEN_DBLSEMI)
  {
    throw "Expected ';;'";
  }

  ++lexer;

  if (*lexer != Parser::TOKEN_ID ||
      TransLucid::get<u32string>(lexer->getValue()) != U"entries")
  {
    throw "Expected \"entries\"";
  }

  ++lexer;

  if (*lexer != Parser::TOKEN_EQUALS)
  {
    throw "Expected =";
  }

  ++lexer;

  size_t numDims = index.size();

  ArrayInit array = parse_array_init(s, lexer, end, numDims, 1);

  if (*lexer != Parser::TOKEN_DBLSEMI)
  {
    throw "Expected ';;'";
  }

  //first count all the dimensions
  std::vector<size_t> max(numDims, 0);

  count_dims(max, array, 0);

  using std::placeholders::_1;

  //set up the dimensions to index it by
  //eval the dimensions
  std::vector<Constant> indexVals;
  std::transform(index.begin(), index.end(),
    std::back_inserter(indexVals),
    std::bind(std::mem_fun(&System::evalExpr), &s, _1)
  );

  //get the dim indices
  for (size_t i = 0; i != indexVals.size(); ++i)
  {
    m_bounds.push_back
    (std::make_pair(
      s.getDimensionIndex(indexVals.at(i)),
      max.at(i)
    ));
  }

  m_array.initialise(m_bounds);

  fill_array{max, s}(m_array.begin(), array, numDims);

#if 0
  //make the variance tuple
  tuple_t variance;
  mpz_class a = 0;
  for (const auto& bound : m_bounds)
  {
    mpz_class b = bound.second - 1;
    std::cerr << "bounds are " << bound.first << ": " 
      << 0 << ".." << b << std::endl;
    variance.insert(std::make_pair(bound.first,
      Types::Range::create(Range(&a, &b))));
  }

  m_variance = variance;

  //set up the multipliers for indexing the array
  m_multipliers.insert(m_multipliers.end(), m_bounds.size(), 0);
  size_t prev = 1;
  auto muliter = m_multipliers.rbegin(); 
  auto bounditer = m_bounds.rbegin();
  while (muliter != m_multipliers.rend())
  {
    *muliter = prev;
    prev = prev * bounditer->second;
    ++muliter;
    ++bounditer;
  }
#endif
}

Constant
FileArrayInFn::applyFn(const std::vector<Constant>& args) const
{
  return Constant();
}

Constant
FileArrayInFn::applyFn(const Constant& arg) const
{
  if (arg.index() == TYPE_INDEX_USTRING)
  {
    try
    {
      return Types::Hyperdatons::create
      (
        new FileArrayInHD(Types::String::get(arg), m_system),
        TYPE_INDEX_INHD
      );
    }
    catch (...)
    {
      //something went wrong, couldn't open the file, bad format or something
      return Types::Special::create(SP_CONST);
    }
  }
  else
  {
    return Types::Special::create(SP_CONST);
  }
}

Constant
FileArrayOutFn::applyFn(const std::vector<Constant>& args) const
{  
  #if 0
  if 
  (
    args.size() == 3 && 
    args[0].index() == TYPE_INDEX_USTRING &&
    args[1].index() == TYPE_INDEX_INTMP &&
    args[2].index() == TYPE_INDEX_INTMP
  )
  {
    try
    {
      return Types::Hyperdatons::create
      (
        new FileArrayOutHD
        (
          Types::String::get(args[0]), 
          Types::Intmp::get(args[1]),
          Types::Intmp::get(args[2]),
          m_system
        ),
        TYPE_INDEX_OUTHD
      );
    }
    catch(...)
    {
      return Types::Special::create(SP_CONST);
    }
  }
  else
  {
    return Types::Special::create(SP_CONST);
  }
  #endif
  return Types::Special::create(SP_CONST);
}

Constant
FileArrayOutFn::applyFn(const Constant& arg) const
{
  if (arg.index() != TYPE_INDEX_USTRING)
  {
    return Types::Special::create(SP_CONST);
  }
  return Types::Hyperdatons::create
  (
    new FileArrayOutHD
    (
      Types::String::get(arg),
      m_system
    ),
    TYPE_INDEX_OUTHD
  );
}

Tuple
FileArrayInHD::variance() const
{
  //return m_variance;
  return m_array.variance();
}

Constant
FileArrayInHD::get(const Context& k) const
{
  return m_array.get(k);

#if 0
  //this is the hard one
  //lookup the bounds dimensions in the context, and convert that to an
  //index

  //assume that the dimensions are correct
  auto boundsiter = m_bounds.begin();
  auto muliter = m_multipliers.begin();
  size_t index = 0;
  while (boundsiter != m_bounds.end())
  {
    auto dim = boundsiter->first;
    const auto& value = k.lookup(dim);
    mpz_class next = (get_constant_pointer<mpz_class>(value) * *muliter);
    index += next.get_ui();
    ++boundsiter;
    ++muliter;
  }

  return m_data[index];
#endif
}


FileArrayOutHD::FileArrayOutHD
(
  const u32string& file, 
  System& system
)
: OutputHD(1), m_file(file), m_system(system)
{
  #if 0
  m_file.open(utf32_to_utf8(file));

  if (!m_file.is_open())
  {
    throw "Could not open output file";
  }

  m_array = new ArrayNHD<mpz_class, 2>
  (
    {m_height, m_width},
    {Types::Dimension::create(DIM_ARG0), Types::Dimension::create(DIM_ARG1)},
    system,
    static_cast<Constant(*)(const mpz_class&)>(&Types::Intmp::create),
    &Types::Intmp::get
  );
  #endif
}

Tuple
FileArrayOutHD::variance() const
{
  return Tuple();
}

void
FileArrayOutHD::commit()
{
  //the array where everything is stored, assuming only one region at the
  //moment
  const ArrayHD& array = *m_regions.at(0).second;

  const Constant* current = array.begin();

  std::ofstream os(utf32_to_utf8(m_file).c_str());

  os << "indexedby = {";

  auto& bounds = array.bounds();

  auto iter = bounds.begin();
  os << m_system.printDimension(iter->first);
  ++iter;

  while (iter != bounds.end())
  {
    os << ", " << iter->first;
    ++iter;
  }

  os << "};;" << std::endl;

  os << "entries = ";

  auto idents = m_system.lookupIdentifiers();
  WS* print = idents.lookup(U"CANONICAL_PRINT");
  Context k = m_system.getDefaultContext();
  printEntries(os, bounds.begin(), bounds.end(), current, print, k);

  os << ";;" << std::endl;
}

void
FileArrayOutHD::put(const Context& t, const Constant& c)
{
  //m_array.put(t, c);

  for (const auto& region : m_regions)
  {
    if (tupleApplicable(region.first, t))
    {
      region.second->put(t, c);
      break;
    }
  }
}

void
FileArrayOutHD::addAssignment(const Tuple& region)
{
  //for now only allow one assignment
  if (m_regions.size() == 1)
  {
    return;
  }

  std::unique_ptr<ArrayHD> array(new ArrayHD);

  //initialise the array
  std::vector<std::pair<dimension_index, size_t>> bounds;

  for (const auto& dim : region)
  {
    switch (dim.second.index())
    {
      case TYPE_INDEX_INTMP:
      //let's just make a region 0..value now
      {
        const mpz_class& val = get_constant_pointer<mpz_class>(dim.second);
        bounds.push_back({dim.first, val.get_ui()});
      }
      break;

      case TYPE_INDEX_RANGE:
      //zero to maximum
      {
        const Range& val = get_constant_pointer<Range>(dim.second);
        if (val.upper() == nullptr)
        {
          return ;
        }
        bounds.push_back({dim.first, val.upper()->get_ui()});
      }
      break;

      default:
      return;
    }
  }

  array->initialise(bounds);

  m_regions.push_back(std::make_pair(region, array.get()));

  array.release();
}

class FileInCreateWS : public WS
{
  public:
  FileInCreateWS(System& s)
  : m_system(s)
  {}

  Constant
  operator()(Context& k)
  {
    return Types::BaseFunction::create(FileArrayInFn(m_system));
  }

  private:
  System& m_system;
};

class FileOutCreateWS : public WS
{
  public:
  FileOutCreateWS(System& s)
  : m_system(s)
  {}

  Constant
  operator()(Context& k)
  {
    return Types::BaseFunction::create(FileArrayOutFn(m_system));
  }

  private:
  System& m_system;
};

//the following functions will create the appropriate file hyperdaton
//which contains an appropriate fstream object and then wrap it up
//as a constant
#if 0
inline Constant
open_input_file(type_index ti, const u32string& file)
{
  FileInputHD* in = new FileInputHD(file);

  return Types::Hyperdatons::create(in, ti);
}

inline Constant
open_output_file(type_index ti, const u32string& file)
{
  //TODO implement me
  return Types::Special::create(SP_ERROR);
}

inline Constant
open_io_file(type_index ti, const u32string& file)
{
  //TODO implement me
  return Types::Special::create(SP_ERROR);
}

struct FileOpener
{
  FileOpener(type_index in, type_index out, type_index io)
  : m_in(in), m_out(out), m_io(io)
  {
  }

  Constant
  operator()(const Constant& file, const Constant& mode)
  {
    if (mode.index() != TYPE_INDEX_INTMP || 
        file.index() != TYPE_INDEX_USTRING)
    {
      return Types::Special::create(SP_TYPEERROR);
    }

    const mpz_class& intmode = get_constant_pointer<mpz_class>(mode);
    const u32string& sfile = get_constant_pointer<u32string>(file);

    switch (intmode.get_ui())
    {
      case 1:
      //input
      return open_input_file(m_in, sfile);
      break;

      case 2:
      //output
      return open_output_file(m_out, sfile);
      break;

      case 3:
      //io
      return open_io_file(m_out, sfile);
      break;

      default:
      return Types::Special::create(SP_DIMENSION);
    }
  }

  type_index m_in, m_out, m_io;
};
#endif


void
init_file_hds(System& s)
{
  //don't know about this stuff
  //this is probably the old wrong stuff
  //type_index in, out, io;

  //in = s.getTypeIndex(U"inhd");
  //out = s.getTypeIndex(U"outhd");
  //io = s.getTypeIndex(U"iohd");

  //s.registerFunction(U"fileopen", 
  //  make_function_type<2>::type(
  //    FileOpener(in, out, io)
  //  )
  //);
  //don't know about above

  s.addEquation(U"file_array_in_hd", new FileInCreateWS(s));
  s.addEquation(U"file_array_out_hd", new FileOutCreateWS(s));
}

}
