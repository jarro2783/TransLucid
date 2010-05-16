/* Directory system main.
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

#if 0

#include <tl/expr_parser.hpp>
#include <tl/interpreter.hpp>
#include <iostream>
#include <ltdl.h>
#include <boost/program_options.hpp>
#include <glibmm/miscutils.h>
#include "directory_system.hpp"
#include <tl/fixed_indexes.hpp>
#include <tl/utility.hpp>
#include <string>
#include <tl/parser.hpp>
#include <tl/tuple_parser.hpp>

namespace TL = TransLucid;
namespace po = boost::program_options;

#endif

int
main(int argc, char** argv)
{
  #if 0
  //TL::Parser::Header h;
  //TL::Parser::HeaderGrammar<TL::Parser::string_type::const_iterator> hg(h);
  //TL::Parser::ExprGrammar<std::u32string::const_iterator> parser(h);
  //TL::Parser::ExprGrammar<TL::Parser::string_type::const_iterator> parser(h);
  //TL::Parser::ExprGrammar<std::string::const_iterator> parser(h);
  //TL::Parser::TupleGrammar<TL::Parser::string_type::const_iterator> tuple;
  //parser.set_context_perturb(tuple);
  //tuple.set_expr(parser);

  using boost::assign::list_of;

  setlocale(LC_ALL, "");

  std::string input;

  po::options_description desc("tl options");
  desc.add_options()
    ("help,h", "show this message")
    ("input,i", po::value<std::string>(), "input directory")
    ("verbose,v", "verbose output")
    ("library-path",
     po::value<std::vector<std::string> >(),
     "add a library search path")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("input"))
  {
    input = vm["input"].as<std::string>();
  }
  else
  {
    std::cerr << "an input directory must be specified" << std::endl;
    return 1;
  }

  TL::DirectoryParser::DirectorySystem system;
  TL::Tuple context;

  if (vm.count("verbose"))
  {
    //system.verbose();
    std::clog << "running in source directory: " <<
    Glib::get_current_dir() + "/" + input << std::endl;
  }

  if (vm.count("library-path"))
  {
    std::vector<std::string> paths =
      vm["library-path"].as<std::vector<std::string> >();
    BOOST_FOREACH(const std::string& s, paths)
    {
      system.addLibrarySearchPath(s);
    }
  }

  typedef std::pair<TL::Constant, TL::Tuple> ValueContextPair;

  std::vector<ValueContextPair> evaluated;

  bool evaluate = true;
  try
  {
    if (!system.parseSystem(input))
    {
      //std::cerr << system.errorCount() << " errors parsing input: "
      //"demands not evaluated" << std::endl;
      evaluate = false;
    }
  }
  catch (const char* c)
  {
    std::cerr << "exception parsing system: " << c << std::endl;
  }

  if (evaluate)
  {
    system.evaluateSystem(std::back_inserter(evaluated));

    //TL::TypeRegistry& registry = system.typeRegistry();
    #if 0
    BOOST_FOREACH(ValueContextPair& p, evaluated)
    {
      const TL::Constant& v = p.first;
      std::cout << "type index: " << v.index() << std::endl;
      switch (v.index())
      {
        case TL::TYPE_INDEX_INTMP:
        std::cout << "intmp<"
                  << v.value<TL::Intmp>().value()
                  << ">"
                  << std::endl;
        break;

        case TL::TYPE_INDEX_USTRING:
        std::cout << "ustring<"
                  << v.value<TL::String>().value()
                  << ">"
                  << std::endl;
        break;

        case TL::TYPE_INDEX_SPECIAL:
        std::cout << "special<"
                  << v.value<TL::Special>().value()
                  << ">"
                  << std::endl;
        break;

        case TL::TYPE_INDEX_UCHAR:
        std::cout << "uchar<";
        v.value<TL::Char>().print(std::cout, TL::Tuple());
        std::cout << ">" << std::endl;
        break;

        default:
        std::cout << "Don't know how to print this" << std::endl;
      }
      #if 0
      //const TL::TypeManager* m = registry.findType(p.first.index());
      //m->print(std::cout, p.first, p.second);
      TL::tuple_t k;
      k[TL::DIM_ID] = TL::generate_string("PRINT");
      k[TL::DIM_VALUE] = p.first;
      TL::Constant s = system(TL::Tuple(k)).first;
      if (s.index() != TL::TYPE_INDEX_USTRING)
      {
        //std::cout << "oops";
      }
      else
      {
        std::cout << s.value<TL::String>().value();
      }
      std::cout << std::endl;
      #endif
    }
    #endif
  }

  return evaluate ? 0 : 1;

  #endif
  return 0;
}
