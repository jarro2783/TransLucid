/* Interactive system main.
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
#include "interactive.hpp"
#include <tl/interpreter.hpp>
#include <boost/program_options.hpp>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace po = boost::program_options;
#endif

int
main(int argc, char *argv[])
{
  #if 0
  setlocale(LC_ALL, "");

  TLInteractive::System system;

  po::options_description desc("tl options");
  desc.add_options()
    ("header,h",
     po::value<std::string>(),
     "add external header for parsing")
    ("library-path",
     po::value<std::vector<std::string>>(),
     "add a library search path")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("header"))
  {
    std::cout << "parsing header “"
              << vm["header"].as<std::string>()
              << "”"
              << std::endl;
    system.parseHeader(vm["header"].as<std::string>());
  }

  std::cout << "TransLucid " PACKAGE_VERSION << std::endl;

  system.run();
  #endif
  #warning fix interactive
  return 0;
}
