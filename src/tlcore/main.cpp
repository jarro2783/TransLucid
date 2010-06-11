/* Core TransLucid application
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

#include <boost/program_options.hpp>
#include <iostream>
#include "tlcore.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  po::options_description desc("tlcore options");
  desc.add_options()
    ("help,h", "show this message")
    ("input", po::value<std::string>(), "input file")
    ("output", po::value<std::string>(), "output file")
    ("reactive", po::value<std::string>(), "reactive system")
    ("verbose", po::value<std::string>(), "verbose output")
    ("version", po::value<std::string>(), "show version")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  TransLucid::TLCore::TLCore tlcore;
  
  if (vm.count("verbose"))
  {
    tlcore.verbose(true);
  }

  if (vm.count("reactive"))
  {
    tlcore.reactive(true);
  }

  tlcore.run();

  return 0;
}
