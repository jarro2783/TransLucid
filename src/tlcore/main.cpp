/* Core TransLucid application main file.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <boost/program_options.hpp>
#include <iostream>
#include "tlcore.hpp"

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  setlocale(LC_ALL, "");

  po::variables_map vm;
  po::options_description desc("tlcore options");
  desc.add_options()
    ("demands", "demand system")
    ("help,h", "show this message")
    ("input", po::value<std::string>(), "input file")
    ("output", po::value<std::string>(), "output file")
    ("reactive", "reactive system")
    ("verbose", "verbose output")
    ("version", "show version")
  ;

  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error parsing command line arguments: " << 
      e.what() << std::endl;
    std::cerr << desc << std::endl;

    return -1;
  }

  if (vm.count("help"))
  {
    std::cerr << desc << std::endl;
    return 0;
  }

  if (vm.count("version"))
  {
    std::cerr << "tlcore " << PACKAGE_VERSION << std::endl;
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
